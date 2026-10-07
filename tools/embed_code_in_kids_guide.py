#!/usr/bin/env python3
"""Copies the REAL sketch files into guide-kids.html so the guide can never drift from the code.
Run after editing any code:  sh tools/make_arduino_sketches.sh && python3 tools/embed_code_in_kids_guide.py
Markers in guide-kids.html look like:  <!--CODE:AI_Companion/config.h|config.h tab--> ... <!--/CODE-->
"""
import re, html, pathlib
ROOT = pathlib.Path(__file__).resolve().parent.parent
GUIDE = ROOT / "guide-kids.html"
KEYWORDS = set("void int bool static const if else for while return switch case break continue true false size_t float char "
               "uint32_t uint8_t int16_t int32_t volatile nullptr sizeof enum class struct unsigned long short double auto default".split())
TOKEN = re.compile(r'(//.*$)|("(?:\\.|[^"\\])*")|(\b0x[0-9A-Fa-f]+\b|\b\d+(?:\.\d+)?[fFuU]?\b)|(\b[A-Za-z_]\w*\b)(\s*\()?')

def highlight_line(line):
    if line.lstrip().startswith("#"):
        m = re.match(r'(\s*)(#\s*\w+)(.*)', line)
        if m:
            indent, directive, rest = m.groups()
            return html.escape(indent) + f'<span class="pp">{html.escape(directive)}</span>' + highlight_line(rest)
    out, pos = [], 0
    for m in TOKEN.finditer(line):
        out.append(html.escape(line[pos:m.start()]))
        com, st, num, ident, paren = m.groups()
        if com:   out.append(f'<span class="cm">{html.escape(com)}</span>')
        elif st:  out.append(f'<span class="st">{html.escape(st)}</span>')
        elif num: out.append(f'<span class="nm">{html.escape(num)}</span>')
        else:
            if ident in KEYWORDS: out.append(f'<span class="kw">{ident}</span>' + (html.escape(paren) if paren else ""))
            elif paren:           out.append(f'<span class="fnm">{ident}</span>' + html.escape(paren))
            else:                 out.append(html.escape(ident))
        pos = m.end()
    out.append(html.escape(line[pos:]))
    return "".join(out)

def block(path, label):
    text = (ROOT / "arduino" / path).read_text()
    code = "\n".join(highlight_line(l) for l in text.rstrip("\n").split("\n"))
    return (f'<div class="codewrap"><span class="fn">{html.escape(label)}</span><button class="copy">Copy</button>'
            f'<pre class="big"><code>{code}</code></pre></div>')

s = GUIDE.read_text()
def sub(m):
    return f'<!--CODE:{m.group(1)}|{m.group(2)}-->' + block(m.group(1), m.group(2)) + '<!--/CODE-->'
s, n = re.subn(r'<!--CODE:([^|>]+)\|([^>]+)-->.*?<!--/CODE-->', sub, s, flags=re.S)
GUIDE.write_text(s)
print(f"Embedded {n} code blocks")
