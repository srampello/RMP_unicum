#pragma once
#include <Arduino.h>
const char WEB_PAGE[] PROGMEM=R"HTML(
<!doctype html><html lang="es"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>RMP UNICUM</title><style>
:root{color-scheme:dark}body{margin:0;background:#09080d;color:#faf8ff;font:16px system-ui}main{max-width:760px;margin:auto;padding:28px}h1{margin-bottom:5px}h1 span{color:#b388ff}.card{background:#17131f;border:1px solid #3a2b50;border-radius:16px;padding:22px;margin:18px 0}label{display:block;margin:12px 0}input,select,button{font:inherit;padding:10px;border-radius:8px;border:1px solid #594375;background:#211a2d;color:white}input{width:110px}button{cursor:pointer;background:#7434c3;margin:6px 8px 6px 0}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(190px,1fr));gap:10px}small{color:#cabbdc}#msg{white-space:pre-wrap}
</style><main><h1>RMP <span>UNICUM</span></h1><small>Configuración de estrategias · ESP32 WROOM</small>
<section class="card">Robot detenido. Los 12 sensores se habilitan al salir de este modo. El Wi-Fi se apaga al iniciar la marcha; la página se desconectará.<p id="status">Cargando…</p></section>
<form class="card" id="form"><label>Estrategia <select name="strategy" id="strategy"><option value="1">1 · Conservadora</option><option value="2">2 · Equilibrada</option><option value="3">3 · Rápida</option></select></label>
<div class="grid"><label>PWM base <input name="base" type="number" min="0" max="255" required></label><label>PWM recta <input name="straight" type="number" min="0" max="255" required></label><label>PWM máximo <input name="max" type="number" min="0" max="255" required></label><label>Kp <input name="kp" type="number" min="0" max="0.2" step="0.001" required></label><label>Kd <input name="kd" type="number" min="0" max="2" step="0.001" required></label></div>
<label>Línea <select name="white"><option value="1">Blanca sobre negro</option><option value="0">Negra sobre blanco</option></select></label><small>Valores iniciales para pruebas. Calibrar y ajustar en la pista.</small><p><button>Guardar estrategia</button></p></form>
<section class="card"><button id="start" type="button">Iniciar en 3 segundos</button><button id="exit" type="button">Salir y leer sensores</button><p id="msg"></p></section>
<script>
let data;const form=document.querySelector('#form'),msg=document.querySelector('#msg');
function fill(){let p=data.strategies[Number(form.strategy.value)-1];for(let k of ['base','straight','max','kp','kd'])form.elements[k].value=p[k]}
async function load(){data=await(await fetch('/status')).json();form.strategy.value=data.strategy;form.white.value=data.white?'1':'0';fill();document.querySelector('#status').textContent='Calibración: '+(data.calibrated?'guardada':'pendiente')+' · PWM: '+(data.motorReady?'listo':'error');document.querySelector('#start').disabled=!data.calibrated||!data.motorReady}
form.strategy.onchange=fill;
form.onsubmit=async e=>{e.preventDefault();try{let r=await fetch('/config',{method:'POST',body:new URLSearchParams(new FormData(form))});msg.textContent=await r.text();if(r.ok)await load()}catch(e){msg.textContent='No se pudo guardar. Revisar conexión.'}};
for(let action of ['start','exit'])document.querySelector('#'+action).onclick=async()=>{try{msg.textContent=await(await fetch('/'+action,{method:'POST'})).text()}catch(e){msg.textContent='Conexión cerrada; comprobar estado del robot.'}};
load().catch(()=>msg.textContent='No se pudo consultar el robot.');
</script></main></html>
)HTML";
