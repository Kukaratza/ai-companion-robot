// Checks Breadboard Buddy's wiring picture against include/config.h.
// Run:  node tools/verify_breadboard_design.js guide-breadboard.html include/config.h
// The picture is the Makeronics BB-102: strip 1 (the brain) and strip 2 (the parts), rails cut after column 30.
const fs=require('fs');
const html=fs.readFileSync(process.argv[2],'utf8'), cfg=fs.readFileSync(process.argv[3],'utf8');
const pin=n=>+cfg.match(new RegExp('#define '+n+'\\s+(\\d+)'))[1];
const want={'mic SCK':pin('PIN_MIC_SCK'),'mic WS':pin('PIN_MIC_WS'),'mic SD':pin('PIN_MIC_SD'),'amp BCLK':pin('PIN_SPK_BCLK'),'amp LRC':pin('PIN_SPK_LRC'),'amp DIN':pin('PIN_SPK_DIN'),
  'tft CS':pin('PIN_TFT_CS'),'tft RESET':pin('PIN_TFT_RST'),'tft DC':pin('PIN_TFT_DC'),'tft SDI':pin('PIN_TFT_MOSI'),'tft SCK':pin('PIN_TFT_SCK')};
const btnPin=pin('PIN_BUTTON');
const a=html.indexOf("  const FREE="), b=html.indexOf("  /* ---------- rendering ---------- */");
const stubs=`
const YO=110,S2=536,DY=S2;const Y={};Object.entries({bt:146,rt:168,a:214,b:236,c:258,d:280,e:302,f:356,g:378,h:400,i:422,j:444,rb:490,bb:512}).forEach(([k,v])=>{Y[k]=v+YO;Y[k+'2']=v+YO+S2});
const X=c=>70+(c-1)*22;const pos=(c,r)=>[X(c),Y[r]];
const COL={yellow:'y',blue:'b',green:'g',red:'r',gray:'k',purple:'p',black:'x',teal:'t',orange:'o'};
const el=()=>({}),txt=()=>({}),gP={},gW={},gR={};
`;
const D=new Function(stubs+html.slice(a,b)+"\nreturn {STEPS,FREE,pinCol,MIC,MIC0,AMP,AMP0,TFT,TFT0,LT,Y,X,BTN};")();
const SPLIT=+html.match(/const SPLIT=(\d+)/)[1];
let fail=0;const ok=(c,m)=>{if(!c)fail++;console.log((c?'ok  : ':'FAIL: ')+m)};
const label=c=>D.FREE[c-2];
for(const [n,c] of Object.entries(D.pinCol)) ok(label(c)===n,`brain column ${c} is printed "${label(c)}" on the picture (expected "${n}")`);
const modPin=c=>c>=D.MIC0&&c<D.MIC0+6?['mic',D.MIC[c-D.MIC0]]:c>=D.AMP0&&c<D.AMP0+7?['amp',D.AMP[c-D.AMP0]]:c>=D.TFT0&&c<D.TFT0+14?['tft',D.TFT[c-D.TFT0]]:['none',''];
const wires=[];D.STEPS.forEach((s,i)=>s.items.forEach(it=>{if(it.k==='w')wires.push({...it,step:i+1})}));
const isHole=e=>Array.isArray(e)&&typeof e[1]==='string';
const RAIL=/^(bt|rt|rb|bb)2?$/;
let signals=0;
for(const w of wires){
  const [ac,ar]=isHole(w.a)?w.a:[null,null],[bc,br]=isHole(w.b)?w.b:[null,null];
  if(ar==='j'&&br==='b2'){const m=modPin(bc),key=m.join(' '),g=label(ac);
    if(key in want){signals++;ok(String(want[key])===g,`${key}: starts at brain pin ${g}, config.h says GPIO${want[key]}`)}
    if(key==='tft VCC')ok(g==='5V','screen VCC is fed from the brain 5V pin (got '+g+')');
  }
  if(ar==='j'&&br==='bb'&&bc===ac)ok(label(ac)==='G','ground wire starts at the G pin');
  if(ar==='j'&&br==='rb'&&bc===ac)ok(label(ac)==='3V3','red-rail wire starts at a 3V3 pin');
  if(ar==='bb'&&br==='bt2')ok(bc===ac,'blue rail of strip 1 is bridged straight down to the blue rail of strip 2');
  if(ar==='rb'&&br==='rt2')ok(bc===ac,'red rail of strip 1 is bridged straight down to the red rail of strip 2');
  if(ar==='rt2'&&br==='b2')ok(['mic VDD','tft LED'].includes(modPin(bc).join(' ')),'red (3.3V) rail feeds only mic VDD and screen LED ('+modPin(bc).join(' ')+')');
  if(ar==='b2'&&br==='bt2')ok(['mic GND','mic L/R','amp GND','tft GND'].includes(modPin(ac).join(' ')),'blue rail takes only GND-type pins ('+modPin(ac).join(' ')+')');
  if(ar==='d2'&&br==='d2'){const k=new Set([modPin(ac).join(' '),modPin(bc).join(' ')]);ok(k.has('tft VCC')&&k.has('amp VIN'),'the short red wire joins the screen VCC column and the amp VIN column (row d)')}
  // the rails are cut after column SPLIT: every rail hole must be on the powered half
  for(const e of [w.a,w.b])if(isHole(e)&&RAIL.test(e[1]))ok(e[0]<=SPLIT,`rail hole col ${e[0]} (${e[1]}) is on the powered half (<= ${SPLIT})`);
}
ok(signals===11,`checked all 11 signal wires against config.h (found ${signals})`);
const btnWire=wires.find(w=>w.grp==='button'&&isHole(w.b)&&w.b[1]==='j');
ok(btnWire&&label(btnWire.b[0])===String(btnPin),`button NO wire goes to brain pin ${btnWire&&label(btnWire.b[0])}, config.h says GPIO${btnPin}`);
const cWire=wires.find(w=>w.grp==='button'&&isHole(w.b)&&w.b[1]==='bb');
ok(cWire,'button C wire goes to the blue rail under the brain');
const lanes=wires.filter(w=>w.lane!=null);
ok(new Set(lanes.map(w=>w.lane)).size===lanes.length,`${lanes.length} lane wires all have their own lane`);
// geometry: every wire becomes a few straight segments; count real crossings and overlaps
const P=e=>isHole(e)?[D.X(e[0]),D.Y[e[1]]]:e;
const segs=w=>{const p=P(w.a),q=P(w.b);if(w.lane==null)return [[p,q]];
  const l=w.lane;return [[p,[p[0],l]],[[p[0],l],[q[0],l]],[[q[0],l],q]]};
const cr=(p,q,r,s)=>{const o=(a,b,c)=>Math.sign((b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]));
  const d1=o(p,q,r),d2=o(p,q,s),d3=o(r,s,p),d4=o(r,s,q);
  if(d1&&d2&&d3&&d4&&d1!==d2&&d3!==d4)return 'cross';
  if(!d1&&!d2&&!d3&&!d4){ // collinear: overlap with length?
    const ax=Math.abs(q[0]-p[0])>=Math.abs(q[1]-p[1])?0:1,lo=Math.max(Math.min(p[ax],q[ax]),Math.min(r[ax],s[ax])),hi=Math.min(Math.max(p[ax],q[ax]),Math.max(r[ax],s[ax]));
    if(hi-lo>1)return 'overlap'}
  return null};
let cross=0,pairs=[];
for(let i=0;i<wires.length;i++)for(let j=i+1;j<wires.length;j++){
  const A=segs(wires[i]),Bs=segs(wires[j]);let hit=null;
  for(const [p,q] of A)for(const [r,s] of Bs){const c=cr(p,q,r,s);if(c)hit=c}
  if(hit){cross++;pairs.push(`${hit}: step ${wires[i].step} (${wires[i].grp}) x step ${wires[j].step} (${wires[j].grp})`)}}
console.log('     crossings:',cross,pairs);
ok(cross<=2,'at most 2 wire crossings');
const holes={};let dup=0;
for(const w of wires)for(const e of [w.a,w.b])if(isHole(e)&&!RAIL.test(e[1])){const k=e.join('/');holes[k]=(holes[k]||0)+1}
for(const [k,v] of Object.entries(holes))if(v>1){dup++;console.log('     hole used twice:',k)}
const rh={};for(const w of wires)for(const e of [w.a,w.b])if(isHole(e)&&RAIL.test(e[1])){const k=e.join('/');rh[k]=(rh[k]||0)+1}
for(const [k,v] of Object.entries(rh))if(v>1){dup++;console.log('     rail hole used twice:',k)}
ok(dup===0,'no breadboard hole is used by two wires');
ok(wires.every(w=>[w.a,w.b].every(e=>!isHole(e)||['j','b2','d2'].includes(e[1])||RAIL.test(e[1]))),'all wire ends are in row j (free row), row b or d of strip 2, or a rail');
const spans=[['mic',D.MIC0,D.MIC0+5],['amp',D.AMP0,D.AMP0+6],['screen',D.TFT0,D.TFT0+13]];
let col=false;for(let i=0;i<spans.length;i++)for(let j=i+1;j<spans.length;j++)if(spans[i][1]<=spans[j][2]&&spans[j][1]<=spans[i][2])col=true;
ok(!col,'mic, amp and screen use different columns on strip 2');
ok(Math.max(...spans.map(s=>s[2]))<=SPLIT,`strip 2 parts end at column ${Math.max(...spans.map(s=>s[2]))} (rails are cut after ${SPLIT})`);
const used=[...Object.values(want),btnPin];ok(new Set(used).size===used.length,'no GPIO number is used twice in config.h: '+used.join(','));
console.log(fail?`\n${fail} FAILED`:'\nall design checks passed');process.exit(fail?1:0);
