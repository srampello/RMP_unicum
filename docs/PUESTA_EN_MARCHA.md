# Puesta en marcha v0.2

## Arduino IDE

1. Instalar **esp32 by Espressif Systems 3.x**. URL de Gestor de placas:
   https://espressif.github.io/arduino-esp32/package_esp32_index.json.
2. Placa **ESP32C3 Dev Module**, **USB CDC On Boot: Enabled**, flash según placa
   (4 MB habitual), partición Default. No usar selección ESP32 Dev Module WROOM.
3. Abrir cada `.ino` desde su carpeta homónima con todos sus `.h`. No agrupar
   sketches. Monitor USB **115200**, terminación Nueva línea/Ambos NL y CR.
4. La opción USB CDC está comprobada por el código: deshabilitada produce error
   deliberado de compilación, porque GPIO20/21 son S2/S3 del MUX.

## Probar antes de marchar

1. Motor sin alimentar, interruptor STBY abierto. Verificar 5 V/3V3/VM 6 V y GND
   común. Revisar nuevo pinout; desconectar cableado de la versión WROOM.
2. Alimentar MicroStart a **3V3**, comprobar SIGNAL=LOW con STOP y HIGH≈3,3 V con
   START. Confirmar pinout físico del módulo. No alimentarlo a 5 V con SIGNAL directo.
3. Cargar `RMP_UNICUM_SENSORES`. C0..C11 deben seguir S01..S12 izquierda→derecha.
   Blanco baja raw, negro lo sube. AP RMP_UNICUM_TEST activo para comprobar lectura
   con Wi-Fi encendido; el sketch no sirve una web y nunca mueve motores.
4. Probar las tres posiciones del selector: elegido cerca de 0, otros altos.
   Monitor informa posición 1/2/3; 0=inválido. C15 a GND. No dejar receptor flotante.
5. Ruedas levantadas y alimentación VM regulada: cargar prueba motores. MicroStart
   debe estar LOW y luego recibir START; cerrar habilitación STBY. Enviar `l 80`,
   `r 80`, `b 80`. Positivo debe avanzar; cambiar cables o `INVERT_LEFT/RIGHT` si
   hace falta y sincronizar headers. `b -80` prueba reversa. Cada orden caduca en
   2 s; `stop`, BOOT o MicroStart LOW paran. MicroStart solo habilita: este sketch
   **no arranca motores automáticamente** cuando llega START.
6. Probar STOP y medir rueda libre al deshabilitar STBY; verificar consumos y picos.

## Firmware principal

1. Cargar `RMP_UNICUM`; selector en 1 y MicroStart STOP. Arranca con motores a cero.
2. Conectar Wi-Fi **RMP_UNICUM**, clave **Unicum2026**, abrir **http://192.168.4.1**.
   Cambiar clave en Config.h si corresponde. Wi-Fi no se apaga durante la carrera.
3. Calibrar: BOOT largo ≥1,2 s, USB `cal` o “Calibrar 8 s” en web. Mover manualmente
   todos los sensores por ambos colores durante ocho segundos, sin mover motores.
4. Revisar barras 0..1000 y raw ADC. `sensors` USB muestra min/max. Calibración
   debe ser válida en los 12 canales, rango≥150. Repetir ante cambios de altura/luz.
5. Seleccionar estrategia conservadora. Calibración válida + selector estable +
   MicroStart LOW → estado ARMADO. Si está desarmado por una parada local, usar
   “Armar”, USB `arm` o BOOT corto con MicroStart STOP.
6. Colocar el robot con la barra sobre línea; enviar START. Firma LOW 50 ms antes
   y HIGH estable 20 ms después. Si se enciende con SIGNAL alto, no arranca. No hay
   demora adicional de 3 s; valor actual START_DELAY_MS=0, ajustar si reglamento exige.
7. STOP remoto baja STBY y detiene. No hace falta mantener IR apuntado en marcha.
   Tras STOP, el MicroStart puede requerir rearme/ciclo de alimentación según su
   funcionamiento; no agregar un bypass en la placa.

MicroStart sirve para START/STOP, **no para encender el rail de batería**. El
interruptor general sigue existiendo. Interruptor STBY abierto deja driver apagado;
no se detecta en software. Antes de cerrarlo, mandar STOP y detener el controlador.

## Comandos USB

| Comando | Acción |
| --- | --- |
| `cal` | Calibrar ocho segundos, MicroStart STOP y robot detenido |
| `arm` | Armar para el siguiente START del MicroStart |
| `stop` | Desarmar y poner PWM a cero |
| `white` / `black` | Polaridad, detenido/STOP; persiste en NVS |
| `status` | JSON de estado completo |
| `sensors` | Lecturas, min/max, selector y MicroStart |

No hay comando `start` ni arranque forzado desde web. Perfil a usar lo determina
el selector 1P3T; editar otro perfil en web no lo selecciona para correr. Guardar
con MicroStart STOP; confirmar mensaje de estado tras “orden en cola”.

## Ajuste en pista

Probar primero con PWM bajo pero suficiente para mover N20, perfil 1, piso seguro.
Ajustar polaridad/orden/dirección antes de Kp/Kd. Luego ganancias y aumento de
velocidad. Medir `cycleUs/overruns`: objetivo tick 5 ms, no garantía de frecuencia
constante en hardware. Si hay excesos/jitter, corregir lectura/telemetría o período
antes de subir velocidad. RPM nominales no permiten calcular PWM preciso sin carga.

STOP de MicroStart deshabilita potencia y deja rueda libre; no garantiza detener
físicamente el robot al instante. Con línea perdida y MicroStart HIGH, PWM cero
frena; tras 150 ms queda desarmado hasta nueva preparación. Wi-Fi desconectado no
es paro de emergencia: conserva control autónomo y MicroStart.
