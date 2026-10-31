# Tourne UI icons

49 vector line icons for the chrome of an app: channel kinds, header buttons, the message box, the user panel, the message hover bar, window buttons. I drew them for my Discord theme, where every icon Discord draws is swapped for one of these, but they're plain SVGs and fit anywhere.

See them all on the [icon packs page](https://user-tourne.github.io/Tourne-Style/icons.html).

## The look

- 24 x 24 grid, 2-unit strokes, square line ends, sharp corners, no fills: the same square look as the rest of Tourne'Style.
- Sharp at any size, since they're vectors; at 16 and 24 px every line sits on the grid.
- Single colour: the stroke is `currentColor`, so they take whatever colour the text around them has.

In my Discord theme they're painted in a gradient from green on the left to red on the right, plain green under the pointer, and the gradient the other way round with a slight shadow when the button is on. A CSS mask does that with any colour you like:

```css
.icon {
  width: 24px; height: 24px;
  background: linear-gradient(90deg, #66ea97, #e57474);
  -webkit-mask: url(svg/channels.svg) center / contain no-repeat;
  mask: url(svg/channels.svg) center / contain no-repeat;
}
.icon:hover { background: #66ea97; }
```

## What's in it

| Where | Icons |
|---|---|
| Channel kinds | text, announcement, voice, stage, forum, rules, thread, locked, media |
| Server rows | events, channels, members, boosts, browse, home, friends, dms, group-add |
| Header | threads, bell, pin, inbox, help, search, invite, minimize, maximize, close |
| Message box | plus, gift, gif, sticker, emoji, apps, send |
| User panel | mic, mic-off, headphones, headphones-off, settings, gamepad, chevron |
| Hover bar and menus | reaction, reply, forward, more, edit, trash, check |

## Licence

My icons are [CC BY-NC-SA 4.0](../../LICENSE-CC-BY-NC-SA-4.0.txt): use them and change them, credit me (Tourne, USER-TOURNE), don't sell them, and share what you make from them under the same licence.
