"""Validar C3, canal ADC1, MUX, USB CDC y sketches independientes."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
src=root/'firmware/UNiCUM/RMP_UNICUM'
cfg=(src/'Config.h').read_text()
array=lambda key:[int(v) for v in re.search(key+r'\[[^]]*\]\s*=\s*\{([^}]+)',cfg)[1].split(',')]
scalar=lambda key:int(re.search(r'\b'+key+r'=(\d+)',cfg)[1])
channels=array('SENSOR_CHANNELS');select=array('MUX_SELECT_PINS');strategy=array('STRATEGY_CHANNELS')
motor=[scalar(n) for n in ('AIN1','AIN2','PWMA','BIN1','BIN2','PWMB')]
pins=select+motor+[scalar('MUX_SIG'),scalar('MICROSTART_PIN'),scalar('CAL_BUTTON')]
assert len(set(pins))==len(pins),'GPIO compartido'
assert set(pins)<={0,1,2,3,4,5,6,7,8,9,10,20,21}
assert scalar('MUX_SIG') in {0,1,2,3,4},'ADC1 requerido'
assert channels==list(range(12)) and strategy==[12,13,14]
assert not set(select+motor)&{9,18,19},'BOOT/USB reservado'
assert not set(motor)&{2,8,9},'Motores sin strapping'
assert 'ARDUINO_USB_CDC_ON_BOOT' in cfg
for ino in root.rglob('*.ino'):
 assert ino.parent.name==ino.stem,ino
 for h in ('Config.h','Hardware.h','StartGate.h'):
  assert (ino.parent/h).read_bytes()==(src/h).read_bytes(),(ino,h)
 assert 'BluetoothSerial' not in ino.read_text()
print('OK: 13 GPIO incluyendo BOOT; ADC1; MUX C0..11 y selector C12..14; 3 sketches C3.')
