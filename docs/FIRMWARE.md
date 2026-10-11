# Firmware v0.1

## Sensado

12 canales directos de `Config.h`. ADC 12 bits, una conversión descartada y cuatro
muestras promediadas por canal. El firmware no lee sensores mientras el AP Wi-Fi
está activo. El tick se solicita cada 5000 µs (~200 Hz); `cycleUs` mide su ejecución
y `overruns` cuenta ticks que duran más de 5 ms. No se garantiza 200 Hz real hasta
medir en hardware: ADC, radio y scheduler afectan el período. Ajustar antes de
subir velocidad; Kd se aplica a diferencia por ciclo, no por segundo.

Calibración manual con robot quieto: guardar extremos durante 8 s, rango mínimo
150 ADC en **cada** canal. Persistencia NVS con marca de validez y validación al
cargar. NVS guarda perfiles numéricos, polaridad y calibración; recalibrar si cambian
pines, electrónica o altura. `unicum-v1` es el namespace para esta configuración.

Normalización por canal 0..1000: 1000 = color de línea elegido. En línea blanca
se invierte lectura calibrada, porque ADC blanco es bajo. Posición por promedio
ponderado entre 0 y 11000; centro **5500**, entre S06 y S07. Un pico de al menos
250 normalizados señala línea; pesos >50 descartan ruido bajo. Umbral ajustable
en `Config.h`. No identifica meta/cruces: una barra totalmente sobre color de
línea entrega centro; una barra sin línea inicia frenado.

## Seguimiento PD (Ki=0)

Error=posición−5500. Error positivo requiere girar a derecha:

`correccion = Kp*error + Kd*derivada_filtrada`

`PWM_izq = base + correccion`, `PWM_der = base - correccion`.

Filtro derivada 0,7 valor previo + 0,3 diferencia actual. PWM de seguimiento
limitado a 0..máximo; **no invierte motores para buscar la línea**. Base sube
suavemente 0,8 PWM/tick y baja 2 PWM/tick; rectas usan incremento cuando
|error|<400 y |diferencia|<100. Valores iniciales sin validación de pista.

| Perfil | Base | Recta | Máximo | Kp | Kd |
| --- | --- | --- | --- | --- | --- |
| Conservadora | 80 | 95 | 130 | 0,035 | 0,10 |
| Equilibrada | 105 | 120 | 170 | 0,040 | 0,14 |
| Rápida | 130 | 145 | 200 | 0,045 | 0,18 |

Son **tres configuraciones del mismo seguidor**, no rutas programadas ni volantazo.
No usar estas ganancias como ajuste final para un Pololu cuyo modelo no conocemos.
Orden recomendado: comprobar giro/dirección, suficiente PWM para iniciar marcha,
Kp para corregir y Kd para amortiguar, luego rectas. Mantener margen corriente/térmica.

## Estados y parada

Inicio detenido. Calibración manual → detenido. Inicio con calibración válida →
cuenta atrás 3 s → verificar línea → marcha. Cualquier botón durante marcha,
calibración o cuenta atrás detiene. En ausencia de línea frena **inmediatamente**;
si no reaparece por 150 ms queda detenido hasta otro arranque. Si reaparece antes,
retoma suavemente. No hace giro de búsqueda a ciegas.

Entrar Wi-Fi siempre detiene y suspende ADC. Salir Wi-Fi desactiva AP/controlador,
espera 50 ms con motores quietos y vuelve a lectura. No se apaga Bluetooth al
arrancar; ADC2 comparte recurso con Wi-Fi, no con SPP. La desconexión de cualquier
enlace no funciona como paro de emergencia.

STBY está habilitado por hardware, no por firmware: PWM cero usa short-brake.
Abrir STBY coloca salidas en alta impedancia. No hay lectura del interruptor STBY,
así que el software no sabe si está abierto. Tras abrirlo en marcha, también
detener por botón/comando antes de volver a cerrarlo para evitar reanudar movimiento.

## Cambios y límites

Configuración de hardware en `Config.h`; ejecutar `tools/sync_headers.py` para
actualizar copias de pruebas. Motor A=izquierda, B=derecha. PWM 20 kHz/8 bits,
API `ledcAttach`/`ledcWrite` de core 3.x. Serial/SPP limita entrada a 32 caracteres
por iteración; las respuestas de comandos siguen pudiendo añadir latencia.

No hay encoder, control de velocidad física, medición de corriente, lectura de
batería, OTA ni telemetría inalámbrica continua. Si se necesita **Wi-Fi + doce
canales en carrera**, cambiar MCU (por ejemplo S3 con ADC1 suficiente y pines
expuestos), añadir ADC externos o cambiar a lectura RC. Cambiar solo firmware
no soluciona ADC2/Wi-Fi del WROOM; no se propone MUX por el requisito del proyecto.
