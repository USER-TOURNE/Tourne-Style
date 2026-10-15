/*
 * Tourne Big Files, a Vencord plugin
 * Copyright (c) 2026 Tourne
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// Sends files bigger than Discord's upload limit, without Nitro, by cutting
// them into pieces that fit and sending each piece as an ordinary attachment.
// The last message carries a small list of the pieces (name.tbf.json). Anyone
// with this plugin gets a card there to play the file in chat or save it whole;
// anyone without it sees the pieces (name.001, name.002, ...) and a line saying
// how to join them with 7-Zip. Nothing leaves Discord unless you also turn on
// "Send as a link", which uploads to a file host instead.

import "./styles.css";

import { NavContextMenuPatchCallback } from "@api/ContextMenu";
import { definePluginSettings } from "@api/Settings";
import { Button } from "@components/Button";
import ErrorBoundary from "@components/ErrorBoundary";
import { classNameFactory } from "@utils/css";
import { insertTextIntoChatInputBox, sendMessage } from "@utils/discord";
import definePlugin, { OptionType, PluginNative } from "@utils/types";
import type { Channel, CloudUpload as TCloudUpload, Message, ModalAction, RenderModalProps } from "@vencord/discord-types";
import { CloudUploadPlatform } from "@vencord/discord-types/enums";
import { findLazy } from "@webpack";
import { Constants, Menu, Modal, openModal, PermissionsBits, PermissionStore, RestAPI, showToast, SnowflakeUtils, useEffect, useState } from "@webpack/common";

const Native = VencordNative.pluginHelpers.TourneBigFiles as PluginNative<typeof import("./native")>;
const CloudUpload: typeof TCloudUpload = findLazy(m => m.prototype?.trackUploadFinished);
const cl = classNameFactory("vc-tbf-");

const MB = 1024 * 1024;
const PIECE_LINE = /^-# 📦 .+ piece (\d+) of (\d+)$/s;
const PIECE_FILE = /\.\d{3,}$/;
const MANIFEST_SUFFIX = ".tbf.json";
const SUPPRESS_NOTIFICATIONS = 1 << 12;
const RUNNABLE = /\.(exe|msi|msix|appx|bat|cmd|com|scr|pif|ps1|vbs|vbe|js|jse|wsf|wsh|hta|lnk|reg|jar|cpl)$/i;

const settings = definePluginSettings({
    pieceSizeMB: {
        type: OptionType.NUMBER,
        description: "Largest piece, in MB. Discord's limit without Nitro is 10 MB, so 9.5 leaves room.",
        default: 9.5,
    },
    piecesIn: {
        type: OptionType.SELECT,
        description: "Where the pieces go in a server (in DMs they always go in the chat)",
        options: [
            { label: "A thread, so the channel only shows the file's card", value: "thread", default: true },
            { label: "The channel itself", value: "chat" },
        ],
    },
    hidePieces: {
        type: OptionType.BOOLEAN,
        description: "Hide the pieces' attachments (the card at the end plays or saves the whole file)",
        default: true,
    },
    previewLimitMB: {
        type: OptionType.NUMBER,
        description: "Play videos, music and pictures in chat up to this size, in MB (bigger ones are saved first)",
        default: 300,
    },
    saveFolder: {
        type: OptionType.STRING,
        description: "Save joined files in this folder (empty: ask where each time)",
        default: "",
    },
    linkHost: {
        type: OptionType.SELECT,
        description: "Send as a link: also offer uploading to a file host and sending its link (the file leaves Discord)",
        options: [
            { label: "Off", value: "off", default: true },
            { label: "Catbox (keeps files, up to 200 MB)", value: "catbox" },
            { label: "Litterbox (Catbox for a while, up to 1 GB)", value: "litterbox" },
            { label: "My own uploader (ShareX .sxcu)", value: "custom" },
        ],
    },
    litterboxTime: {
        type: OptionType.SELECT,
        description: "How long Litterbox keeps a file",
        options: [
            { label: "1 hour", value: "1h" },
            { label: "12 hours", value: "12h" },
            { label: "24 hours", value: "24h" },
            { label: "72 hours", value: "72h", default: true },
        ],
    },
    linkAutoSend: {
        type: OptionType.BOOLEAN,
        description: "Send the link right away (off: put it in the message box)",
        default: false,
    },
    customUploader: {
        type: OptionType.COMPONENT,
        component: CustomUploaderSetting,
    },
}).withPrivateSettings<{
    catboxUserHash?: string;
    custom?: { name: string; url: string; fileFormName: string; args: Record<string, string>; headers: Record<string, string>; responsePath: string; };
}>();

// ---------------------------------------------------------------- helpers

interface Piece {
    c: string; // channel id
    m: string; // message id
    f: string; // file name as Discord stored it
    s: number; // size
    h: string; // sha-256
}

interface Manifest {
    app: "Tourne Big Files";
    v: 1;
    name: string;
    size: number;
    type: string;
    sha256: string;
    pieces: Piece[];
}

const sleep = (ms: number) => new Promise<void>(r => setTimeout(r, ms));

function sizeText(bytes: number) {
    if (bytes >= 1024 * MB) return (bytes / 1024 / MB).toFixed(2) + " GB";
    if (bytes >= MB) return (bytes / MB).toFixed(1) + " MB";
    return Math.max(1, Math.round(bytes / 1024)) + " KB";
}

function errorText(err: any) {
    return String(err?.body?.message ?? err?.message ?? err);
}

async function sha256(bytes: Uint8Array) {
    const digest = await crypto.subtle.digest("SHA-256", bytes as BufferSource);
    return [...new Uint8Array(digest)].map(b => b.toString(16).padStart(2, "0")).join("");
}

function mimeOf(name: string) {
    const ext = name.toLowerCase().split(".").pop() ?? "";
    const types: Record<string, string> = {
        mp4: "video/mp4", m4v: "video/mp4", mov: "video/quicktime", webm: "video/webm", mkv: "video/x-matroska",
        mp3: "audio/mpeg", m4a: "audio/mp4", ogg: "audio/ogg", opus: "audio/ogg", wav: "audio/wav", flac: "audio/flac",
        png: "image/png", jpg: "image/jpeg", jpeg: "image/jpeg", gif: "image/gif", webp: "image/webp", avif: "image/avif",
        zip: "application/zip", "7z": "application/x-7z-compressed", pdf: "application/pdf",
    };
    return types[ext] ?? "application/octet-stream";
}

function oneLine(text: string) {
    return text.replace(/[\r\n]+/g, " ");
}

// Asks Discord again when it's busy: messages can be sent about one a second.
async function withRetry<T>(work: () => Promise<T>): Promise<T> {
    for (let attempt = 1; ; attempt++) {
        try {
            return await work();
        } catch (err: any) {
            const busy = err?.status === 429 || err?.status >= 500;
            if (!busy || attempt >= 6) throw err;
            await sleep(((err?.body?.retry_after ?? attempt) + 0.25) * 1000);
        }
    }
}

function uploadToDiscord(file: File, channelId: string) {
    return new Promise<TCloudUpload>((resolve, reject) => {
        const upload = new CloudUpload({ file, isThumbnail: false, platform: CloudUploadPlatform.WEB }, channelId);
        upload.on("complete", () => resolve(upload));
        upload.on("error", () => reject(new Error("Discord didn't take the upload.")));
        upload.upload();
    });
}

async function postWithFile(channelId: string, content: string, file: File, flags = 0) {
    const upload = await withRetry(() => uploadToDiscord(file, channelId));
    const res = await withRetry(() => RestAPI.post({
        url: Constants.Endpoints.MESSAGES(channelId),
        body: {
            channel_id: channelId,
            content,
            flags,
            nonce: SnowflakeUtils.fromTimestamp(Date.now()),
            sticker_ids: [],
            type: 0,
            attachments: [{ id: "0", filename: upload.filename, uploaded_filename: upload.uploadedFilename }],
        },
    }));
    return res.body as { id: string; attachments: { filename: string; }[]; };
}

async function postText(channelId: string, content: string) {
    const res = await withRetry(() => RestAPI.post({
        url: Constants.Endpoints.MESSAGES(channelId),
        body: { channel_id: channelId, content, nonce: SnowflakeUtils.fromTimestamp(Date.now()), sticker_ids: [], type: 0 },
    }));
    return res.body as { id: string; };
}

// ---------------------------------------------------------------- the job being sent

type PickedFile = { path: string; name: string; size: number; };
type Mode = "pieces" | "link";

interface Job {
    mode: Mode;
    file: PickedFile;
    channel: Channel;
    started: boolean;
    stop: boolean;
    text: string;
    done: number;
    total: number;
    end?: "sent" | "failed";
}

// One file is sent at a time. The window showing it can be closed and opened
// again from the upload button's menu while the pieces keep going.
let job: Job | null = null;
const jobListeners = new Set<() => void>();

function updateJob(patch: Partial<Job>) {
    if (!job) return;
    job = { ...job, ...patch };
    jobListeners.forEach(f => f());
}

function useJob() {
    const [, redraw] = useState(0);
    useEffect(() => {
        const f = () => redraw(n => n + 1);
        jobListeners.add(f);
        return () => { jobListeners.delete(f); };
    }, []);
    return job;
}

function pieceSize() {
    return Math.floor(Math.min(500, Math.max(1, settings.store.pieceSizeMB || 9.5)) * MB);
}

function canThread(channel: Channel) {
    return !!channel.guild_id && (channel.type === 0 || channel.type === 5)
        && PermissionStore.can(PermissionsBits.CREATE_PUBLIC_THREADS, channel)
        && PermissionStore.can(PermissionsBits.SEND_MESSAGES_IN_THREADS, channel);
}

function hostName() {
    const host = settings.store.linkHost;
    const { custom } = settings.store;
    if (host === "catbox") return "catbox.moe";
    if (host === "litterbox") return "litterbox.catbox.moe";
    if (!custom?.url) return "";
    try {
        return custom.name || new URL(custom.url).host;
    } catch {
        return custom.url;
    }
}

async function startSending(mode: Mode, channel: Channel) {
    if (job && !job.end) {
        openJobWindow();
        return;
    }
    if (mode === "link" && settings.store.linkHost === "custom" && !settings.store.custom?.url) {
        showToast("Import a ShareX uploader (.sxcu) in Tourne Big Files' settings first.", "failure");
        return;
    }
    const file = await Native.pickFile();
    if (!file) return;
    if (mode === "pieces" && file.size <= pieceSize()) {
        showToast("That file fits in one message: attach it the usual way.", "message");
        return;
    }
    job = { mode, file: { ...file, name: oneLine(file.name) }, channel, started: false, stop: false, text: "", done: 0, total: 0 };
    openJobWindow();
}

function openJobWindow() {
    openModal(props => <ErrorBoundary><JobWindow modalProps={props} /></ErrorBoundary>);
}

function JobWindow({ modalProps }: { modalProps: RenderModalProps; }) {
    const j = useJob();
    if (!j) return null;
    const { file, mode } = j;
    const count = Math.ceil(file.size / pieceSize());
    const thread = settings.store.piecesIn === "thread" && canThread(j.channel);
    const running = j.started && !j.end;

    const actions: ModalAction[] = !j.started
        ? [{ text: mode === "link" ? "Upload" : "Send", variant: "primary", onClick: () => (mode === "link" ? sendAsLink() : sendInPieces()) }]
        : running
            ? [{ text: "Stop", variant: "critical-primary", disabled: j.stop || mode === "link", onClick: () => updateJob({ stop: true, text: "Stopping after this piece" }) },
            { text: "Hide", variant: "secondary", onClick: modalProps.onClose }]
            : [{ text: "Close", variant: "primary", onClick: modalProps.onClose }];

    return (
        <Modal {...modalProps} title={mode === "link" ? "Send as a link" : "Send in pieces"} actions={actions}>
            <div className={cl("ask")}>
                {!j.started && mode === "pieces" && <>
                    <p>Send <b>{file.name}</b> ({sizeText(file.size)}) as {count} pieces of up to {sizeText(pieceSize())}?</p>
                    <p>{thread ? "The pieces go in a thread, and this channel gets the file's card." : "The pieces go in this chat, then the file's card."} Each piece is a silent message, about one a second.</p>
                </>}
                {!j.started && mode === "link" && <>
                    <p>Upload <b>{file.name}</b> ({sizeText(file.size)}) to <b>{hostName()}</b> and send its link?</p>
                    <p>The file leaves Discord: anyone with the link can download it{settings.store.linkHost === "litterbox" ? `, for ${settings.store.litterboxTime}` : ""}.</p>
                </>}
                {j.started && <>
                    <p><b>{file.name}</b></p>
                    <p className={j.end === "failed" ? cl("error") : undefined}>{j.text}</p>
                    {j.total > 0 && <progress className={cl("bar")} value={j.done} max={j.total} />}
                    {running && <p className={cl("dim")}>You can hide this window: sending goes on. Open it again from the upload button's menu.</p>}
                </>}
            </div>
        </Modal>
    );
}

// ---------------------------------------------------------------- sending in pieces

async function sendInPieces() {
    if (!job || job.started) return;
    const { file, channel } = job;
    const { name } = file;
    const size = pieceSize();
    const count = Math.ceil(file.size / size);
    const digits = Math.max(3, String(count).length);
    updateJob({ started: true, text: "Checking the file", total: count, done: 0 });
    try {
        const wholeSha = await Native.hashFile(file.path);

        let piecesChannel = channel.id;
        let threadId = "";
        if (settings.store.piecesIn === "thread" && canThread(channel)) {
            try {
                const starter = await postText(channel.id, `📦 Sending **${name}** (${sizeText(file.size)}) in ${count} pieces...`);
                const thread = await withRetry(() => RestAPI.post({
                    url: `/channels/${channel.id}/messages/${starter.id}/threads`,
                    body: { name: `📦 ${name}`.slice(0, 100), auto_archive_duration: 1440 },
                }));
                threadId = piecesChannel = thread.body.id;
            } catch (err) {
                updateJob({ text: `No thread (${errorText(err)}), so the pieces go here.` });
            }
        }

        const pieces: Piece[] = [];
        for (let i = 0; i < count; i++) {
            if (job?.stop) throw new Error(`Stopped after ${i} of ${count} pieces. The ones sent can be deleted.`);
            updateJob({ text: `Sending piece ${i + 1} of ${count}`, done: i });
            const offset = i * size;
            const bytes = await Native.readPiece(file.path, offset, Math.min(size, file.size - offset));
            const pieceName = `${name}.${String(i + 1).padStart(digits, "0")}`;
            const posted = await postWithFile(piecesChannel, `-# 📦 ${name} piece ${i + 1} of ${count}`,
                new File([bytes as BlobPart], pieceName, { type: "application/octet-stream" }), SUPPRESS_NOTIFICATIONS);
            pieces.push({ c: piecesChannel, m: posted.id, f: posted.attachments?.[0]?.filename ?? pieceName, s: bytes.byteLength, h: await sha256(bytes) });
            await sleep(1100);
        }

        updateJob({ text: "Sending the file's card", done: count });
        const manifest: Manifest = { app: "Tourne Big Files", v: 1, name, size: file.size, type: mimeOf(name), sha256: wholeSha, pieces };
        const first = pieces[0].f, last = pieces[count - 1].f;
        const lines = [
            `📦 **${name}** · ${sizeText(file.size)} · ${count} pieces${threadId ? ` in <#${threadId}>` : ""}`,
            `-# Sent with Tourne Big Files. No plugin? Save every piece in one folder, then open ${first} in 7-Zip and pick Combine files (or run: copy /b ${first}+...+${last} "${name}").`,
        ];
        await postWithFile(channel.id, lines.join("\n"),
            new File([JSON.stringify(manifest)], name + MANIFEST_SUFFIX, { type: "application/json" }));
        updateJob({ end: "sent", text: `Sent in ${count} pieces.` });
        showToast(`Sent ${name}.`, "success");
    } catch (err) {
        updateJob({ end: "failed", text: `Couldn't send it: ${errorText(err)}` });
        showToast(`Couldn't send ${name}.`, "failure");
    }
}

// ---------------------------------------------------------------- sending as a link

async function sendAsLink() {
    if (!job || job.started) return;
    const { file, channel } = job;
    const host = settings.store.linkHost;
    const { custom } = settings.store;
    updateJob({ started: true, text: `Uploading to ${hostName()}` });
    try {
        const link = await Native.uploadToHost(file.path, host === "custom"
            ? { kind: "custom", url: custom!.url, fileFormName: custom!.fileFormName, args: custom!.args, headers: custom!.headers, responsePath: custom!.responsePath }
            : { kind: host as "catbox" | "litterbox", litterboxTime: settings.store.litterboxTime, catboxUserHash: settings.store.catboxUserHash });
        if (settings.store.linkAutoSend) await sendMessage(channel.id, { content: link });
        else insertTextIntoChatInputBox(link + " ");
        updateJob({ end: "sent", text: settings.store.linkAutoSend ? `Sent: ${link}` : `The link is in the message box: ${link}` });
    } catch (err) {
        updateJob({ end: "failed", text: `Upload failed: ${errorText(err)}` });
        showToast(`Couldn't upload ${file.name}.`, "failure");
    }
}

// ---------------------------------------------------------------- receiving

function manifestAttachment(message: Message) {
    return message.attachments?.find(a => a.filename?.endsWith(MANIFEST_SUFFIX));
}

function isPiece(message: Message) {
    return message.attachments?.length === 1 && PIECE_FILE.test(message.attachments[0].filename ?? "")
        && PIECE_LINE.test(message.content ?? "");
}

function readManifest(text: string): Manifest {
    const m = JSON.parse(text);
    const ok = m?.app === "Tourne Big Files" && m.v === 1 && typeof m.name === "string" && Number.isFinite(m.size)
        && /^[0-9a-f]{64}$/.test(m.sha256) && Array.isArray(m.pieces) && m.pieces.length > 0 && m.pieces.length <= 10000
        && m.pieces.every((p: Piece) => /^\d+$/.test(p?.c) && /^\d+$/.test(p?.m) && /^[0-9a-f]{64}$/.test(p?.h) && Number.isFinite(p?.s));
    if (!ok) throw new Error("This card's list of pieces isn't one this plugin can read.");
    return m;
}

// Discord's file links expire, so the pieces' messages are read again to get
// current ones. Pieces sit in order, so this reads forward from the first.
async function currentLinks(m: Manifest, show: (text: string) => void) {
    const links: string[] = new Array(m.pieces.length);
    const byChannel = new Map<string, number[]>();
    m.pieces.forEach((p, i) => byChannel.set(p.c, [...(byChannel.get(p.c) ?? []), i]));
    for (const [channelId, indexes] of byChannel) {
        const wanted = new Map(indexes.map(i => [m.pieces[i].m, i]));
        const ids = [...wanted.keys()].map(BigInt).sort((a, b) => (a < b ? -1 : a > b ? 1 : 0));
        const last = ids[ids.length - 1];
        let after = ids[0] - 1n;
        for (let page = 0; wanted.size && page < 1000; page++) {
            show(`Finding the pieces (${m.pieces.length - wanted.size} of ${m.pieces.length})`);
            const res = await withRetry(() => RestAPI.get({
                url: Constants.Endpoints.MESSAGES(channelId),
                query: { after: after.toString(), limit: 100 },
            }));
            const batch = (res.body ?? []) as { id: string; attachments?: { filename: string; url: string; }[]; }[];
            if (!batch.length) break;
            for (const msg of batch) {
                const id = BigInt(msg.id);
                if (id > after) after = id;
                const i = wanted.get(msg.id);
                if (i === undefined) continue;
                const att = msg.attachments?.find(a => a.filename === m.pieces[i].f) ?? msg.attachments?.[0];
                if (att) {
                    links[i] = att.url;
                    wanted.delete(msg.id);
                }
            }
            if (after >= last) break;
        }
        if (wanted.size) throw new Error(`${wanted.size} of the pieces are gone (deleted, or somewhere you can't see).`);
    }
    return links;
}

type CardState =
    | { step: "idle"; }
    | { step: "busy"; text: string; }
    | { step: "playing"; url: string; }
    | { step: "saved"; path: string; }
    | { step: "error"; text: string; };

function BigFileCard({ message }: { message: Message; }) {
    const attachment = manifestAttachment(message)!;
    const [manifest, setManifest] = useState<Manifest | null>(null);
    const [loadError, setLoadError] = useState("");
    const [state, setState] = useState<CardState>({ step: "idle" });

    useEffect(() => {
        let alive = true;
        Native.fetchText(attachment.url)
            .then(text => alive && setManifest(readManifest(text)))
            .catch(err => alive && setLoadError(errorText(err)));
        return () => { alive = false; };
    }, [attachment.url]);

    useEffect(() => () => {
        if (state.step === "playing") URL.revokeObjectURL(state.url);
    }, [state]);

    if (loadError) return <div className={cl("card")}><span className={cl("error")}>📦 {loadError}</span></div>;
    if (!manifest) return <div className={cl("card")}><span className={cl("dim")}>📦 Reading the file's card...</span></div>;

    const m = manifest;
    const kind = m.type.split("/")[0];
    const canPlay = ["video", "audio", "image"].includes(kind) && m.size <= (settings.store.previewLimitMB || 300) * MB;
    const busy = state.step === "busy";
    const show = (text: string) => setState({ step: "busy", text });

    async function play() {
        try {
            const links = await currentLinks(m, show);
            const parts: Uint8Array[] = [];
            for (let i = 0; i < links.length; i++) {
                show(`Getting piece ${i + 1} of ${links.length}`);
                const bytes = await Native.fetchPiece(links[i]);
                if (await sha256(bytes) !== m.pieces[i].h) throw new Error(`Piece ${i + 1} came back different from what was sent.`);
                parts.push(bytes);
            }
            setState({ step: "playing", url: URL.createObjectURL(new Blob(parts as BlobPart[], { type: m.type })) });
        } catch (err) {
            setState({ step: "error", text: errorText(err) });
        }
    }

    async function save() {
        let token = "";
        try {
            const begun = await Native.beginSave(m.name, settings.store.saveFolder.trim());
            if (!begun) return;
            token = begun.token;
            const links = await currentLinks(m, show);
            for (let i = 0; i < links.length; i++) {
                show(`Saving piece ${i + 1} of ${links.length}`);
                const problem = await Native.appendPiece(token, links[i], m.pieces[i].h);
                if (problem) throw new Error(`Piece ${i + 1}: ${problem}`);
            }
            show("Checking the whole file");
            const done = await Native.finishSave(token, m.sha256);
            token = "";
            if (done.error || !done.path) throw new Error(done.error ?? "Couldn't finish saving.");
            setState({ step: "saved", path: done.path });
        } catch (err) {
            if (token) await Native.abortSave(token);
            setState({ step: "error", text: errorText(err) });
        }
    }

    return (
        <div className={cl("card")}>
            <div className={cl("head")}>
                <span className={cl("name")}>📦 {m.name}</span>
                <span className={cl("dim")}>{sizeText(m.size)} · {m.pieces.length} pieces</span>
            </div>
            {state.step === "playing" && (
                kind === "video" ? <video className={cl("media")} src={state.url} controls autoPlay />
                    : kind === "audio" ? <audio className={cl("audio")} src={state.url} controls autoPlay />
                        : <img className={cl("media")} src={state.url} alt={m.name} />
            )}
            {state.step === "busy" && <span className={cl("dim")}>{state.text}...</span>}
            {state.step === "error" && <span className={cl("error")}>{state.text}</span>}
            {state.step === "saved" && <span className={cl("dim")}>Saved to {state.path}</span>}
            <div className={cl("buttons")}>
                {canPlay && state.step !== "playing" && <Button size="small" disabled={busy} onClick={play}>{kind === "image" ? "Show" : "Play"} here</Button>}
                {state.step !== "saved" && <Button size="small" variant="secondary" disabled={busy} onClick={save}>Save...</Button>}
                {state.step === "saved" && !RUNNABLE.test(state.path) && <Button size="small" onClick={() => Native.openFile(state.path)}>Open</Button>}
                {state.step === "saved" && <Button size="small" variant="secondary" onClick={() => Native.showInFolder(state.path)}>Show in folder</Button>}
            </div>
        </div>
    );
}

// ---------------------------------------------------------------- settings: ShareX uploader

function sxcuPath(url: string) {
    const m = url.match(/\{json:([^}]+)\}/) ?? url.match(/\$json:([^$]+)\$/);
    return m ? m[1].trim() : "";
}

function CustomUploaderSetting() {
    const [custom, setCustom] = useState(settings.store.custom);

    async function importSxcu() {
        const [file] = await DiscordNative.fileManager.openFiles({ filters: [{ name: "ShareX uploader", extensions: ["sxcu"] }] });
        if (!file) return;
        try {
            const sx = JSON.parse(new TextDecoder().decode(file.data));
            if (!/^https?:\/\//i.test(sx?.RequestURL ?? "")) throw new Error("It has no RequestURL.");
            const strings = (o: unknown) => Object.fromEntries(Object.entries(o && typeof o === "object" ? o : {})
                .filter(([k, v]) => v != null && !["__proto__", "constructor", "prototype"].includes(k))
                .map(([k, v]) => [k, String(v)]));
            const next = {
                name: String(sx.Name ?? ""),
                url: String(sx.RequestURL),
                fileFormName: String(sx.FileFormName || "file"),
                args: strings(sx.Arguments),
                headers: strings(sx.Headers),
                responsePath: sx.ResponseType === "Text" ? "" : sxcuPath(String(sx.URL ?? "")),
            };
            settings.store.custom = next;
            setCustom(next);
            showToast(`Imported ${next.name || new URL(next.url).host}.`, "success");
        } catch (err) {
            showToast(`Couldn't read that .sxcu: ${errorText(err)}`, "failure");
        }
    }

    return (
        <div className={cl("setting")}>
            <span>My own uploader: {custom?.url ? <b>{custom.name || custom.url}</b> : "none yet"}</span>
            <div className={cl("buttons")}>
                <Button size="small" variant="secondary" onClick={importSxcu}>Import ShareX uploader (.sxcu)...</Button>
                {custom?.url && <Button size="small" variant="dangerSecondary" onClick={() => { settings.store.custom = undefined; setCustom(undefined); }}>Remove</Button>}
            </div>
        </div>
    );
}

// ---------------------------------------------------------------- plugin

const attachMenu: NavContextMenuPatchCallback = (children, { channel }: { channel: Channel; }) => {
    if (channel.guild_id && !(PermissionStore.can(PermissionsBits.SEND_MESSAGES, channel) && PermissionStore.can(PermissionsBits.ATTACH_FILES, channel))) return;
    if (job?.started && !job.end) {
        children.push(<Menu.MenuItem id="vc-tbf-show" label={`Sending ${job.file.name}...`} action={openJobWindow} />);
        return;
    }
    children.push(<Menu.MenuItem id="vc-tbf-pieces" label="Send a big file (in pieces)" action={() => startSending("pieces", channel)} />);
    if (settings.store.linkHost !== "off")
        children.push(<Menu.MenuItem id="vc-tbf-link" label="Send a big file as a link" action={() => startSending("link", channel)} />);
};

export default definePlugin({
    name: "TourneBigFiles",
    description: "Send files over Discord's upload limit without Nitro: they go in pieces and play or save whole from a card. Right-click the upload (+) button.",
    authors: [{ name: "Tourne", id: 0n }],
    settings,

    contextMenus: {
        "channel-attach": attachMenu,
    },

    patches: [{
        find: "this.renderAttachments(",
        replacement: {
            match: /this\.renderAttachments\((\i)\)/,
            replace: "($self.hideAttachments($1)?null:$&)",
        },
    }],

    hideAttachments(message: Message) {
        if (!message) return false;
        return !!manifestAttachment(message) || (settings.store.hidePieces && isPiece(message));
    },

    renderMessageAccessory({ message }) {
        if (!manifestAttachment(message)) return null;
        return (
            <ErrorBoundary noop>
                <BigFileCard message={message} />
            </ErrorBoundary>
        );
    },
});
