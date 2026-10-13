#!/usr/bin/env python3
"""Diagrama SVG exacto del pinout actual, sin dependencias externas."""
from pathlib import Path
import html,re
root=Path(__file__).resolve().parents[1]
cfg=(root/'firmware/UNiCUM/RMP_UNICUM/Config.h').read_text()
scalar=lambda key:int(re.search(r'\b'+key+r'=(\d+)',cfg)[1])
select=[int(x) for x in re.search(r'MUX_SELECT_PINS\[[^]]*\]=\{([^}]+)',cfg)[1].split(',')]
def txt(x,y,s,size=18,color='#f4efff',anchor='start'):
 return f'<text x="{x}" y="{y}" font-family="sans-serif" font-size="{size}" fill="{color}" text-anchor="{anchor}">{html.escape(s)}</text>'
def box(x,y,w,h,fill='#191321',stroke='#9063d1'):
 return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="12" fill="{fill}" stroke="{stroke}"/>'
def wire(x1,y1,x2,y2):
 return f'<path d="M{x1} {y1} L{x2} {y2}" fill="none" stroke="#c7a4ff" stroke-width="3"/>'
s='<svg xmlns="http://www.w3.org/2000/svg" width="1260" height="1000" viewBox="0 0 1260 1000">'+box(0,0,1260,1000,'#0c0910','#0c0910')
s+=txt(36,50,'RMP UNICUM · C3 + MUX12 + MicroStart',30)+txt(36,84,'N20 6 V / 2000 RPM · Wi-Fi activo · distribución lógica v0.2',19,'#baa2d9')
s+=box(35,120,300,475)+txt(55,155,'Barra QRE1113GR ×12',23)+txt(55,187,'LED: 5 V · salidas: 3,3 V',16,'#baa2d9')
s+=box(395,120,300,475)+txt(415,155,'74HC4067 · VCC 3,3 V',22)+txt(415,187,'EN a GND · C15 a GND',16,'#baa2d9')
for i in range(12):
 y=230+i*27;s+=txt(55,y,f'S{i+1:02d}  →  C{i}',18)+txt(420,y,f'C{i}  ←  S{i+1:02d}',18)+wire(335,y-6,395,y-6)
s+=box(760,120,465,325)+txt(782,155,'TB6612FNG · N20 ×2',24)
rows=[f'AIN1 ← {scalar("AIN1")}    AIN2 ← {scalar("AIN2")}    PWMA ← {scalar("PWMA")}',f'BIN1 ← {scalar("BIN1")}    BIN2 ← {scalar("BIN2")}    PWMB ← {scalar("PWMB")}','Canal A: izquierdo · Canal B: derecho','VCC 3,3 V · VM 6 V regulados','PWM1/10 y AIN/BIN: 10 kΩ a GND','STBY ← SIGNAL por interruptor','STBY LOW: alta impedancia / rueda libre']
for i,t in enumerate(rows):s+=txt(782,197+i*33,t,18)
s+=box(760,475,465,120)+txt(782,511,'DIP switch · 3 vías',23)+txt(782,544,'SW1 / SW2 / SW3 → C12 / C13 / C14',17)+txt(782,576,'Otro terminal de cada SW → GND · solo uno ON',17,'#baa2d9')
s+=wire(695,536,760,536)
s+=box(35,635,660,195)+txt(55,672,'ESP32-C3 Super Mini',26)
s+=txt(55,713,'S0 / S1 / S2 / S3 → GPIO '+ ' / '.join(map(str,select)),20)
s+=txt(55,748,f'SIG → GPIO{scalar("MUX_SIG")} (ADC1) · MicroStart → GPIO{scalar("MICROSTART_PIN")}',20)
s+=txt(55,784,'BOOT GPIO9: calibrar/armar/parar · USB CDC activado',18,'#baa2d9')
s+=wire(545,595,545,635)
s+=box(760,635,465,195)+txt(782,672,'MicroStart JSumo',26)
s+=txt(782,713,'Alimentación: 3,3 V · GND común',19)+txt(782,748,'SIGNAL → GPIO3 y STBY',19)+txt(782,784,'HIGH = START · LOW = STOP',19,'#baa2d9')
s+=wire(695,746,760,746)
s+=box(35,865,1190,98)+txt(55,903,'LiPo 2S → 6 V regulados para motores + 5 V para placa y emisores.',21)
s+=txt(55,938,'3V3 → MUX, receptores, MicroStart y VCC TB6612. Confirmar corriente de bloqueo de N20.',18,'#baa2d9')
s+='</svg>'
(root/'hardware/conexiones.svg').write_text(s)
print('hardware/conexiones.svg actualizado desde Config.h')
