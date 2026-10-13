# Firmware v0.2 · C3 + MUX + MicroStart

## Lectura y telemetría

GPIO0=ADC1_CH0. C0..C11 contienen la barra; C12..C14 el DIP de tres vías. S0..S3
seleccionan binariamente cada canal; esperar 120 µs, descartar una conversión y
promediar dos muestras. Solo la tarea de control accede al MUX/ADC. El servidor
web lee una copia del estado, nunca conmuta el MUX.

Wi-Fi AP permanece activo en calibración y marcha. Ya no se suspende la barra por
usar Wi-Fi: todas las conversiones pasan por ADC1. Web actualiza aproximadamente
4 veces/s; el control tiene período objetivo de 5 ms, independiente del HTTP.
C3 es de un solo núcleo: tareas de radio/scheduler pueden producir jitter; ver
`cycleUs` y `overruns`, y medir período real en pista. Kd es por diferencia de
ciclo, no derivada por segundo. Si se cambia frecuencia, volver a ajustar Kd.

Tarea de control prioridad 3, stack 6144 bytes, cola de ocho órdenes; procesa hasta
cuatro por tick. HTTP/USB envían órdenes; configuración se vuelve a validar cuando
la tarea la procesa. Estado compartido copiado en sección crítica corta. ADC,
motores, perfiles y NVS tienen un solo propietario para evitar carreras de datos.
Las operaciones NVS se hacen detenido. No imprimir sensores desde el control.

## Selector y START/STOP

DIP de tres interruptores independientes mediante los canales libres del MUX: exactamente uno
LOW y dos HIGH. ON conecta su canal a GND (LOW). Todos OFF o más de uno ON bloquean START.
Estado estable 50 ms, consultado cada 25 ms. Perfil de carrera
se captura al START; cambiar el switch en marcha no altera ganancias/velocidades.

MicroStart alimentado a 3,3 V, HIGH=START, LOW=STOP. Su SIGNAL también llega a
STBY del driver: LOW lo deshabilita por hardware aunque el servidor/tarea tarde.
Ese estado es alta impedancia, **no frenado activo**. La latencia/inercia real debe
medirse; no confundir cortar empuje con detener físicamente instantáneamente.

`StartGate` exige **LOW al menos 50 ms y luego HIGH al menos 20 ms**, una sola
vez por secuencia. HIGH al encender/reset no inicia el robot. STOP se aplica sin
retardo de debounce en el firmware y directamente en STBY. No se necesita seguir
apuntando el control remoto después de START: MicroStart mantiene su estado.

Con calibración válida y selector válido, autoarmado al encender/completar
calibración con MicroStart LOW. “Armar” por web, USB `arm` o BOOT corto deja el
robot esperando otra secuencia LOW→HIGH. START se rechaza si no ve línea/selector.
No hay botón web/USB para forzar movimiento, y la web no suplanta MicroStart.

Demora extra por firmware `START_DELAY_MS=0`; si el reglamento exige demora,
ajustar ese parámetro. STOP cancela tanto espera como marcha. Parada web/USB/BOOT
o pérdida prolongada de línea desarma; requieren rearmado y nuevo START. STOP de
MicroStart permite autoarmado para el próximo START válido (según el propio módulo).

## Calibración y control

Calibración manual de 8 s sobre blanco/negro, motores quietos y MicroStart LOW.
Rango mínimo 150 ADC por canal: uno defectuoso invalida todo el arranque. Al iniciar
calibración se invalida marca NVS; al completar correctamente guarda extremos.
Un START durante calibración la cancela. Namespace **unicum-c3-v2**, distinto del
WROOM anterior. Recalibrar si cambia altura, electrónica, canal/pin o pista/luz.

Lectura cruda 0..4095. Normalización 0..1000 por canal: color de línea elegido=1000.
Blanco suele producir ADC bajo, por eso línea blanca invierte lectura. Posición
ponderada 0..11000, centro=5500. Umbral de presencia=250 y pesos >50.
No distingue meta ni cruces; toda barra sobre línea devuelve posición central.

Control PD, Ki=0: error=posición−5500; corrección=Kp×error + Kd×diferencia filtrada.
Filtro derivada=0,7 anterior + 0,3 diferencia. Error positivo: aumentar izquierda
y reducir derecha. PWM de seguimiento solo positivo, 0..máximo (sin giro de búsqueda).
Arranque suave: base sube 0,8/tick y baja 2/tick. Recta si |error|<400 y |delta|<100.

| Estrategia | Base | Recta | Máximo | Kp | Kd |
| --- | --- | --- | --- | --- | --- |
| Conservadora | 80 | 95 | 130 | 0,035 | 0,10 |
| Equilibrada | 105 | 120 | 170 | 0,040 | 0,14 |
| Rápida | 130 | 145 | 200 | 0,045 | 0,18 |

Son semillas, no velocidades verificadas para N20 2000 RPM. Motores sin encoders:
PWM no equivale a velocidad física. Ajustar por rueda/batería/pista. Si falta línea,
PWM va a cero inmediatamente; si vuelve antes de 150 ms, retoma con rampa. Si no,
queda desarmado. Si START permanece HIGH, línea perdida frena activamente mediante
PWM cero; STOP MicroStart baja STBY y deja rueda libre.

## Interfaz y límites

AP RMP_UNICUM, clave inicial Unicum2026, http://192.168.4.1. Muestra raw/señal de
12 canales, min/max (USB/JSON), selector físico, perfil activo, MicroStart, PWM y
tiempos. Web edita perfiles numéricos y polaridad solamente con robot detenido y
MicroStart STOP. Guardado en NVS; cambiar perfil de edición no selecciona carrera.
No carga credenciales de una red externa; el robot genera su propio AP.

No hay BluetoothSerial: C3 soporta BLE, no SPP Classic; no se implementó servicio
BLE en v0.2. USB CDC On Boot obligatorio; no usar UART0 en GPIO20/21 (S2/S3 del MUX) ni GPIO8 como
LED desde otro código. No hay encoders, corriente/batería, OTA ni detección de meta.
Pérdida de Wi-Fi no detiene el robot; START/STOP sigue en MicroStart.
