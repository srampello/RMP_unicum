# Puesta en marcha

## Arduino IDE

1. Instalar **esp32 by Espressif Systems, serie 3.x** en Gestor de placas.
   URL adicional: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`.
2. Seleccionar **ESP32 Dev Module**, puerto USB correcto, flash según placa real
   (4 MB asumidos aquí), **Huge APP (3MB No OTA/1MB SPIFFS)** para el principal.
3. Abrir `.ino` desde su propia carpeta. Los `.h` de esa carpeta son necesarios.
   No reunir sketches diferentes en una carpeta. No necesita QTRSensors externa.
4. Monitor serial **115200**, terminación **Nueva línea** o **Ambos NL y CR**.
   El principal usa API LEDC 3.x; no compila sin cambios en core 2.x.

## Pruebas en orden

1. STBY abierto y motores sin alimentar. Verificar masa común, 5 V, 3V3 y 6 V VM.
   Medir que ninguna salida de receptor supere 3,3 V.
2. Cargar `RMP_UNICUM_SENSORES`: leer las doce columnas 0..4095. Pasar blanco y
   negro por **cada sensor**: blanco debe bajar y negro subir. Comprobar orden
   izquierda/derecha y ruido. No hay Wi-Fi en esta prueba.
3. Levantar ruedas; cargar `RMP_UNICUM_MOTORES`, cerrar STBY y usar `l 80`,
   `r 80`, `b 80`. Positivo debe mover cada rueda hacia el avance. Si hace falta,
   invertir cables o `INVERT_LEFT`/`INVERT_RIGHT` en `Config.h` principal y
   ejecutar `tools/sync_headers.py`. La prueba frena después de 2 s sin comando.
4. Comprobar botones y apertura de STBY; no probar dirección con el robot apoyado.
5. Cargar principal. Robot inicia **detenido y con Wi-Fi apagado**. Iniciar `cal`
   por USB, Bluetooth o mantener B1 1,2 s. Mover manualmente la barra durante
   8 s sobre ambos colores; **los motores no hacen la calibración**.
6. Revisar `sensors`: mínimo/máximo por canal y señal de línea 0..1000. Si un
   sensor tiene rango <150 ADC, se rechaza calibración completa y se bloquea inicio.
   Calibración válida se guarda en NVS. Recalibrar al cambiar altura/pista/luz.
7. Colocar barra sobre línea, apoyar robot y pulsar brevemente B1. Arranca en
   3 s con perfil conservador. Cualquier botón cancela. B2/B3 seleccionan e
   inician perfiles 2/3; ajustar progresivamente, no comenzar con perfil rápido.

## Wi-Fi de configuración

Mantener B2 1,2 s detenido, o comando `wifi`. Conectar a:

- SSID: **RMP_UNICUM_SETUP**.
- Contraseña inicial: **Unicum2026** (cambiar `AP_PASSWORD` en `Config.h`).
- Navegador: **http://192.168.4.1**.

Elegir perfil, PWM base/recta/máximo, Kp/Kd y color de línea. Guardar.
Valores en NVS; condición 0≤base≤recta≤máximo≤255. La web no muestra sensores
en este modo: **ADC2 está ocupado por Wi-Fi**. Salir y utilizar la prueba serial
o `sensors` para medir la barra. “Iniciar” responde y apaga AP antes de la cuenta
atrás; la página pierde conexión por diseño. Para calibrar desde Wi-Fi: salir y
usar B1/USB/Bluetooth. No refrescar buscando recuperar la conexión durante carrera.

## Bluetooth

Bluetooth **Classic SPP**, nombre `RMP_UNICUM_BT`. Usar terminal SPP en Android
o puerto serie Bluetooth en Windows que lo soporte. No es BLE y iOS normalmente
no permite una terminal genérica SPP. `ENABLE_BLUETOOTH=false` permite desactivarlo.
Usa los mismos comandos terminados en nueva línea que USB. No hay autenticación
de aplicación ni cifrado adicional definido aquí; configurar fuera de carrera.

No depender de USB/Bluetooth como parada inmediata garantizada. Parada local:
botones; deshabilitación física: abrir STBY. Pérdida de enlace **no detiene** la
marcha autónoma; pérdida de línea sí. No hay streaming en carrera para no cargar
el lazo. Si se necesita telemetría continua, evaluar tareas/colas y tiempos reales.

## Comandos USB/SPP

| Comando | Acción |
| --- | --- |
| `cal` | Salir de Wi-Fi, motores parados, calibración manual 8 s |
| `1`, `2`, `3` | Seleccionar estrategia detenido; no inicia |
| `start` | Iniciar con perfil elegido tras 3 s; apaga Wi-Fi |
| `stop` | Parar/cancelar; si hay Wi-Fi, salir |
| `white`, `black` | Polaridad de línea; solo detenido/configuración |
| `wifi` | Parar y activar AP; suspender sensores |
| `exit` | Apagar AP y volver detenido a lectura |
| `status` | Estado, perfil, calibración, posición, PWM y tiempo del tick |
| `sensors` | Raw, señal normalizada, mínimo/máximo; solo detenido |

Las órdenes de configuración se pueden usar por Bluetooth/USB; evitar consultas
durante carrera para conservar temporización. `start` no mueve si falta calibración
o PWM; al terminar cuenta atrás cancela si no ve línea.
