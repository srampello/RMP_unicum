"""Verificar señales, asignación GPIO y sketches independientes."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
src=root/'firmware/UNiCUM/RMP_UNICUM'
cfg=(src/'Config.h').read_text()
array=lambda key:[int(v) for v in re.search(key+r'\[[^]]*\]\s*=\s*\{([^}]+)',cfg)[1].split(',')]
sensors=array('SENSOR_PINS'); buttons=array('BUTTON_PINS')
motor=[int(re.search(r'\b'+name+r'=(\d+)',cfg)[1]) for name in ('AIN1','AIN2','PWMA','BIN1','BIN2','PWMB')]
pins=sensors+buttons+motor
assert len(sensors)==12 and len(buttons)==3
assert len(set(pins))==len(pins),'GPIO compartido'
assert set(sensors)<={4,13,14,25,26,27,32,33,34,35,36,39}
assert not set(pins)&{0,1,2,3,6,7,8,9,10,11,12,37,38}
assert all(p<34 for p in buttons+motor)
for ino in root.rglob('*.ino'):
    assert ino.parent.name==ino.stem,ino
    for h in ('Config.h','Hardware.h'):
        assert (ino.parent/h).read_bytes()==(src/h).read_bytes(),(ino,h)
print('OK: 21 GPIO sin duplicados; 12 ADC; 3 sketches independientes.')
