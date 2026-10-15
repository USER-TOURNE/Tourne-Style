/*
 * Tourne Big Files, a Vencord plugin
 * Copyright (c) 2026 Tourne
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// The parts that touch the disk and the network run here, in Discord's main
// process: files are read one piece at a time and written straight to disk,
// so a 2 GB video never sits in the chat window's memory.

import { createHash, Hash } from "crypto";
import { app, BrowserWindow, dialog, IpcMainInvokeEvent, shell } from "electron";
import { createReadStream, openAsBlob } from "fs";
import { FileHandle, mkdir, open, rename, rm, stat } from "fs/promises";
import { basename, dirname, extname, join } from "path";

const CDN_HOSTS = ["cdn.discordapp.com", "media.discordapp.net"];
const FETCH_TIMEOUT_MS = 2 * 60 * 1000;
const HOST_TIMEOUT_MS = 30 * 60 * 1000;

// Only files picked in the file dialog can be read.
const picked = new Set<string>();

function windowOf(e: IpcMainInvokeEvent) {
    return BrowserWindow.fromWebContents(e.sender) ?? undefined;
}

export async function pickFile(e: IpcMainInvokeEvent) {
    const win = windowOf(e);
    const options: Electron.OpenDialogOptions = { title: "Send a big file", properties: ["openFile"] };
    const result = win ? await dialog.showOpenDialog(win, options) : await dialog.showOpenDialog(options);
    const path = result.filePaths[0];
    if (result.canceled || !path) return null;
    const info = await stat(path);
    picked.add(path);
    return { path, name: basename(path), size: info.size };
}

export async function readPiece(_: IpcMainInvokeEvent, path: string, offset: number, length: number) {
    if (!picked.has(path)) throw new Error("That file wasn't picked.");
    const fh = await open(path, "r");
    try {
        const buffer = Buffer.alloc(length);
        let done = 0;
        while (done < length) {
            const { bytesRead } = await fh.read(buffer, done, length - done, offset + done);
            if (!bytesRead) break;
            done += bytesRead;
        }
        return new Uint8Array(buffer.buffer, buffer.byteOffset, done);
    } finally {
        await fh.close();
    }
}

export function hashFile(_: IpcMainInvokeEvent, path: string) {
    if (!picked.has(path)) throw new Error("That file wasn't picked.");
    return new Promise<string>((resolve, reject) => {
        const hash = createHash("sha256");
        createReadStream(path)
            .on("data", chunk => hash.update(chunk))
            .on("end", () => resolve(hash.digest("hex")))
            .on("error", reject);
    });
}

function checkCdn(url: string) {
    const u = new URL(url);
    if (u.protocol !== "https:" || !CDN_HOSTS.includes(u.hostname)) throw new Error("Not a Discord file link.");
}

async function fetchBytes(url: string) {
    checkCdn(url);
    const res = await fetch(url, { signal: AbortSignal.timeout(FETCH_TIMEOUT_MS) });
    if (!res.ok) throw new Error(`Discord answered ${res.status}`);
    return new Uint8Array(await res.arrayBuffer());
}

export async function fetchPiece(_: IpcMainInvokeEvent, url: string) {
    return fetchBytes(url);
}

export async function fetchText(_: IpcMainInvokeEvent, url: string) {
    return new TextDecoder().decode(await fetchBytes(url));
}

// ---------------------------------------------------------------- saving

interface Saving {
    fh: FileHandle;
    temp: string;
    path: string;
    hash: Hash;
}

const saving = new Map<string, Saving>();
let nextToken = 1;

function safeName(name: string) {
    const clean = basename(name).replace(/[<>:"/\\|?*\x00-\x1f]/g, "_").replace(/[. ]+$/, "").trim();
    return clean || "file";
}

async function freePath(folder: string, name: string) {
    const ext = extname(name);
    const stem = name.slice(0, name.length - ext.length);
    for (let n = 1; ; n++) {
        const path = join(folder, n === 1 ? name : `${stem} (${n})${ext}`);
        try {
            await stat(path);
        } catch {
            return path;
        }
    }
}

export async function beginSave(e: IpcMainInvokeEvent, name: string, folder: string) {
    name = safeName(name);
    let path: string;
    if (folder) {
        await mkdir(folder, { recursive: true });
        path = await freePath(folder, name);
    } else {
        const win = windowOf(e);
        const options: Electron.SaveDialogOptions = { title: "Save the file", defaultPath: join(app.getPath("downloads"), name) };
        const result = win ? await dialog.showSaveDialog(win, options) : await dialog.showSaveDialog(options);
        if (result.canceled || !result.filePath) return null;
        path = result.filePath;
    }
    const temp = path + ".tbfpart";
    const fh = await open(temp, "w");
    const token = String(nextToken++);
    saving.set(token, { fh, temp, path, hash: createHash("sha256") });
    return { token, path };
}

// Fetches one piece, checks it, adds it to the file. "" when it went in.
export async function appendPiece(_: IpcMainInvokeEvent, token: string, url: string, sha256: string) {
    const s = saving.get(token);
    if (!s) return "The save was stopped.";
    try {
        const bytes = await fetchBytes(url);
        if (createHash("sha256").update(bytes).digest("hex") !== sha256) return "A piece came back different from what was sent.";
        await s.fh.write(bytes);
        s.hash.update(bytes);
        return "";
    } catch (err) {
        return String((err as Error)?.message ?? err);
    }
}

export async function finishSave(_: IpcMainInvokeEvent, token: string, sha256: string) {
    const s = saving.get(token);
    if (!s) return { error: "The save was stopped." };
    saving.delete(token);
    await s.fh.close();
    if (s.hash.digest("hex") !== sha256) {
        await rm(s.temp, { force: true });
        return { error: "The joined file doesn't match the one that was sent." };
    }
    await rm(s.path, { force: true });
    await rename(s.temp, s.path);
    return { path: s.path };
}

export async function abortSave(_: IpcMainInvokeEvent, token: string) {
    const s = saving.get(token);
    if (!s) return;
    saving.delete(token);
    await s.fh.close().catch(() => { });
    await rm(s.temp, { force: true });
}

export async function openFile(_: IpcMainInvokeEvent, path: string) {
    return shell.openPath(path);
}

export function showInFolder(_: IpcMainInvokeEvent, path: string) {
    shell.showItemInFolder(path);
}

export function defaultFolder() {
    return join(app.getPath("downloads"), "Tourne Big Files");
}

export function folderOf(_: IpcMainInvokeEvent, path: string) {
    return dirname(path);
}

// ---------------------------------------------------------------- hosts

export interface HostRequest {
    kind: "catbox" | "litterbox" | "custom";
    litterboxTime?: string;
    catboxUserHash?: string;
    url?: string;
    fileFormName?: string;
    args?: Record<string, string>;
    headers?: Record<string, string>;
    responsePath?: string;
}

function pathValue(value: any, path: string) {
    for (const part of path.split(".").map(s => s.trim()).filter(Boolean)) {
        if (value == null) return undefined;
        const index = part.match(/^(.*)\[(\d+)\]$/);
        value = index ? value[index[1]]?.[Number(index[2])] : value[part];
    }
    return value;
}

// Uploads a picked file to a file host and returns its link. The file is
// streamed from disk.
export async function uploadToHost(_: IpcMainInvokeEvent, path: string, req: HostRequest) {
    if (!picked.has(path)) throw new Error("That file wasn't picked.");
    const blob = await openAsBlob(path);
    const form = new FormData();
    let url: string;
    const headers: Record<string, string> = {};
    if (req.kind === "catbox" || req.kind === "litterbox") {
        url = req.kind === "catbox" ? "https://catbox.moe/user/api.php" : "https://litterbox.catbox.moe/resources/internals/api.php";
        form.append("reqtype", "fileupload");
        if (req.kind === "catbox" && req.catboxUserHash) form.append("userhash", req.catboxUserHash);
        if (req.kind === "litterbox") form.append("time", req.litterboxTime || "72h");
        form.append("fileToUpload", blob, basename(path));
    } else {
        if (!req.url || !/^https?:\/\//i.test(req.url)) throw new Error("The custom uploader has no request URL.");
        url = req.url;
        for (const [k, v] of Object.entries(req.args ?? {})) form.append(k, v);
        Object.assign(headers, req.headers ?? {});
        form.append(req.fileFormName || "file", blob, basename(path));
    }
    const res = await fetch(url, { method: "POST", body: form, headers, signal: AbortSignal.timeout(HOST_TIMEOUT_MS) });
    const text = (await res.text()).trim();
    if (!res.ok) throw new Error(`The host answered ${res.status}: ${text.slice(0, 200)}`);
    let link = text;
    if (req.kind === "custom" && req.responsePath) {
        let json: any;
        try {
            json = JSON.parse(text);
        } catch {
            throw new Error("The host didn't answer with JSON.");
        }
        link = String(pathValue(json, req.responsePath) ?? "");
    }
    if (!/^https?:\/\//i.test(link)) throw new Error(`The host didn't send back a link: ${text.slice(0, 200)}`);
    return link;
}
