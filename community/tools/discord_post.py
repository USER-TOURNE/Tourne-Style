"""Posts new community styles to the Discord server's table-styles forum, one post per style, with a GIF of its
live preview from the styles page and the .ttstyle file attached.

  python3 community/tools/discord_post.py find <before> <after>   styles added to index.txt between two commits
  python3 community/tools/discord_post.py find --files a.ttstyle,b.ttstyle
      Writes files=<space-separated list> to $GITHUB_OUTPUT (empty when nothing is new).
  python3 community/tools/discord_post.py post <gif dir> <file.ttstyle>...
      Posts each style through the webhook in $DISCORD_STYLES_WEBHOOK (skipped with a note when it isn't set).
"""
import json, os, re, subprocess, sys, time, urllib.error, urllib.request, uuid

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SETUPS = os.path.join(ROOT, 'community', 'setups')
REPO = 'USER-TOURNE/Tourne-Style'
PAGE = 'https://user-tourne.github.io/Tourne-Style/'
SAFE = re.compile(r'^[A-Za-z0-9_.-]+\.ttstyle$')


def entries(text):
    out = {}
    for line in text.splitlines():
        if not line or line.startswith('#'): continue
        f = line.split('|')
        if len(f) >= 4 and SAFE.match(f[0]): out[f[0]] = {'file': f[0], 'name': f[1], 'author': f[2], 'about': '|'.join(f[3:])}
    return out


def index_at(rev):
    try:
        return subprocess.run(['git', 'show', f'{rev}:community/setups/index.txt'], cwd=ROOT, capture_output=True,
                              text=True, check=True).stdout
    except subprocess.CalledProcessError:
        return ''


def find(args):
    now = entries(open(os.path.join(SETUPS, 'index.txt'), encoding='utf-8').read())
    if args[0] == '--files':
        files = [f for f in args[1].replace(',', ' ').split() if f in now]
    else:
        before, after = args
        if not before.strip('0'): before = f'{after}~1'  # first push of a branch
        old = entries(index_at(before))
        files = [f for f in entries(index_at(after)) if f not in old and f in now]
    print('new styles:', files or 'none')
    with open(os.environ.get('GITHUB_OUTPUT', os.devnull), 'a') as o:
        o.write('files=' + ' '.join(files) + '\n')


def send(url, payload, files):
    bd = uuid.uuid4().hex
    data = (f'--{bd}\r\nContent-Disposition: form-data; name="payload_json"\r\nContent-Type: application/json\r\n\r\n'
            f'{json.dumps(payload)}\r\n').encode()
    for i, (name, kind, blob) in enumerate(files):
        data += (f'--{bd}\r\nContent-Disposition: form-data; name="files[{i}]"; filename="{name}"\r\n'
                 f'Content-Type: {kind}\r\n\r\n').encode() + blob + b'\r\n'
    data += f'--{bd}--\r\n'.encode()
    for _ in range(5):
        req = urllib.request.Request(url + '?wait=true', data=data, method='POST', headers={
            'Content-Type': f'multipart/form-data; boundary={bd}', 'User-Agent': 'Tourne-Style community bot'})
        try:
            return json.load(urllib.request.urlopen(req))
        except urllib.error.HTTPError as e:
            body = e.read().decode(errors='replace')
            if e.code == 429:
                time.sleep(json.loads(body).get('retry_after', 2) + 0.5)
                continue
            raise SystemExit(f'Discord said {e.code}: {body[:300]}')
    raise SystemExit('Discord kept rate-limiting the post')


def post(gif_dir, files):
    url = os.environ.get('DISCORD_STYLES_WEBHOOK', '').strip()
    if not url:
        print('DISCORD_STYLES_WEBHOOK is not set, so nothing was posted.')
        return
    now = entries(open(os.path.join(SETUPS, 'index.txt'), encoding='utf-8').read())
    for f in files:
        e = now.get(f)
        if not e: continue
        about = re.sub(r'#(\d+)', lambda m: f'[#{m.group(1)}](<https://github.com/{REPO}/issues/{m.group(1)}>)', e['about'])
        text = (f"**{e['name']}** by {e['author']}\n{about}\n\n"
                "To try it: right-click Tourne'Table, **My Styles > Style Editor > Community**, or **Open file** and "
                "pick the attached file.\n"
                f"Live on the styles page: <{PAGE}>")
        attach = []
        gif = os.path.join(gif_dir, f + '.gif')
        if os.path.exists(gif):
            attach.append((f[:-len('.ttstyle')] + '.gif', 'image/gif', open(gif, 'rb').read()))
        attach.append((f, 'text/plain', open(os.path.join(SETUPS, f), 'rb').read()))
        payload = {'thread_name': e['name'][:100], 'content': text[:2000], 'allowed_mentions': {'parse': []},
                   'attachments': [{'id': i, 'filename': a[0]} for i, a in enumerate(attach)]}
        r = send(url, payload, attach)
        print(f"{f}: posted ({r.get('channel_id')}, {len(r.get('attachments', []))} files)")


if __name__ == '__main__':
    cmd, rest = sys.argv[1], sys.argv[2:]
    if cmd == 'find': find(rest)
    elif cmd == 'post': post(rest[0], rest[1:])
    else: raise SystemExit(__doc__)
