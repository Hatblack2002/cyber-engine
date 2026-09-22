# CYBER ENGINE — PROYECTO 01
## Especificación Adaptada v1.1 (para continuidad en otro agente)

**Origen:** CYBER ENGINE v1.0 + decisiones de producto del usuario  
**Estado del documento:** EJECUTABLE  
**Fecha de adaptación:** 2026-09-21  
**Propósito:** Entregar a un agente de implementación con contexto completo, sin perder reglas ni progreso real.

---

## 0. CONTRATO CON EL USUARIO (NO NEGOCIABLE)

1. **Diseño visual y 3D del producto final** deben alinearse con el mockup "CYBER COMMAND OS" (estética neón, paneles, terminal, viewport 3D, dashboard).
2. **Los datos NUNCA se inventan.** Cada panel muestra solo lo que el dispositivo puede demostrar.
3. **Dispositivos con privilegios elevados (root, permisos):** más capacidades `AVAILABLE`.
4. **Dispositivos sin privilegios:** mismos paneles y diseño; estados `UNAVAILABLE` / `REQUIRES_PERMISSION` / `REQUIRES_ROOT` / `NOT_IMPLEMENTED` visibles y honestos.
5. **Prohibido** fingir ROOT, TERMUX, escaneo de red, nmap, ARP, packet capture, FPS constantes, listas de procesos inventadas, filesystem libre de `/storage/emulated/0` si Scoped Storage no lo permite.

### Regla fundamental (spec original §5)
> Ningún módulo podrá declarar una capacidad que no pueda demostrar.

Estados legales de capacidad:
`AVAILABLE | PARTIAL | UNAVAILABLE | REQUIRES_PERMISSION | REQUIRES_ROOT | REQUIRES_HARDWARE | NOT_IMPLEMENTED | ERROR`

---

## 1. VISIÓN DE PRODUCTO VS ALCANCE PROYECTO 01

| Capa | Objetivo final (mockup) | Proyecto 01 |
|------|-------------------------|-------------|
| Estética cyber / layout paneles | Sí | Empezar dirección visual |
| Viewport 3D real | Sí | Geometría real mínima (no decoración falsa) |
| Terminal | Sí | Shell real con comandos implementados |
| ROOT badge | Solo si root real | `UNAVAILABLE` si no hay root |
| Mapa de red / nmap / dispositivos LAN | Producto posterior | Fuera de alcance P01 |
| CPU/RAM/batería | Datos de APIs Android | Solo lo expuesto por el sistema |
| Automatización / workflows | Posterior (Lua) | Sandbox Lua mínimo en P01 |

El mockup es **dirección de diseño**, no checklist de features falsas.

---

## 2. PLATAFORMA Y STACK

- **Plataforma:** Android nativo (minSdk 26, target 34)
- **Lenguajes:** C++20 (motor) + Java/Kotlin (Activity fina)
- **Render:** Vulkan (preferido) → OpenGL ES (fallback) → None (error observable)
- **Sin Termux** como dependencia
- **Build:** Gradle + CMake + NDK 26
- **ABIs:** arm64-v8a (prioridad)

---

## 3. ESTADO REAL DEL CÓDIGO (2026-09-21)

### Completado y verificable

| Módulo | Ubicación | Notas |
|--------|-----------|-------|
| Result, Status, IDs, Handles, Span, StringView | `engine/core/` | Tests host OK |
| SystemAllocator | `engine/memory/` | |
| JobSystem (worker pool) | `engine/jobs/` | |
| Diagnostics ring buffer | `engine/diagnostics/` | |
| CapabilityManager | `engine/capabilities/` | Fuente única de verdad |
| EventBus tipado | `engine/events/` | |
| Engine lifecycle | `engine/engine.h` | Transiciones validadas |
| Shell (help, version, capabilities, diagnostics, echo) | `engine/shell/` | Host demo OK |
| IRenderer + factory | `engine/renderer/` | Vulkan = NOT_IMPLEMENTED |
| GLESRenderer (EGL clear + swap) | `engine/renderer/gles/` | Real en Android |
| AndroidBridge + SurfaceView + Choreographer | `android/app/...` | pause/resume/surface |
| APK arm64 | `cyber-engine-v2-debug.apk` | Backend GLES + frames reales |

### Tests host
11 suites (~149 checks) — Result, Status, Handles, Span, Memory, Diagnostics, Jobs, Capabilities, EventBus, Engine, Shell.

### Pendiente (orden obligatorio)

1. Mesh 3D real + cámara (orbit/pan/zoom) + clear distinto de “solo color”
2. UI dashboard (paneles alineados al mockup; contenido gated por CapabilityManager)
3. Terminal embebida en UI Android (wire al Shell)
4. Vulkan backend real + fallback documentado
5. Filesystem (AppPrivate / SAF / estados honestos)
6. System info / process info (solo APIs públicas)
7. Lua VM + sandbox
8. Vertical slice §50 completo

---

## 4. ARQUITECTURA (INVARIABLE)

```
Core → Memory → Jobs → EventBus → CapabilityManager → Renderer → Scene → UI
```

Especializados conectan solo por contratos/EventBus:
Filesystem, Process, Network, AndroidBridge, Lua, Shell.

**Prohibido:** UI → internos de Vulkan; Lua → registries privados; Core → Activity Android.

### Boot sequence (§8)
```
Activity → AndroidBridge.initialize() → Engine::create()
→ Diagnostics → Memory → JobSystem → EventBus → CapabilityManager
→ Renderer (Vulkan|GLES|fail) → Scene → UI → Lua → READY
```
Cada etapa retorna Result verificable. No `tryInitialize(); ignoreFailure();`.

### Lifecycle Android (§31–32)
SurfaceDestroyed ≠ EngineDestroyed.  
Pause: soltar recursos de surface, retener estado del motor.  
Resume: recreate surface/swapchain.

---

## 5. RENDERER

Orden: **Vulkan → GLES → ENGINE_GRAPHICS_UNAVAILABLE**.

Implementado hoy: GLES con EGL, clear color cyber dark, present real, resize, release/recreate surface.

Pendiente: Vulkan completo; geometría 3D; FPS medido (nunca constante 60 falso).

HUD debe mostrar backend y frame count **reales** (ya parcialmente hecho).

---

## 6. CAPABILITY MANAGER (COMPORTAMIENTO POR DISPOSITIVO)

Al detectar:

| Capability | Sin privilegios típicos | Con root / permisos |
|------------|-------------------------|---------------------|
| Vulkan / GLES | Según hardware/API | Igual |
| AppPrivateStorage | AVAILABLE | AVAILABLE |
| SharedStorage | PARTIAL / REQUIRES_PERMISSION | puede subir a AVAILABLE |
| SAF | AVAILABLE tras grant | igual |
| Root | UNAVAILABLE | AVAILABLE solo si detectado de verdad |
| NetworkObservation | básico / PARTIAL | no implica ARP/raw socket |
| ProcessInfo | solo lo que Android expone | no fingir /proc completo |
| Lua | cuando se implemente | sandbox por defecto restrictivo |
| NativePlugins | DISABLED en P01 | DISABLED |

El **diseño de paneles es el mismo** en todos los dispositivos. Solo cambian badges/estados/datos.

---

## 7. SHELL (COMANDOS P01)

Implementados: `help`, `version`, `capabilities`, `diagnostics`, `echo`, `clear`.

Pendientes (solo si hay backend real): `pwd`, `ls`, `cd`, `mkdir`, `cat`, … filesystem; `system`; `process`; `network` (observación básica).

`version` y `capabilities` deben reflejar CapabilityManager y backend real — nunca strings hardcodeados de “Vulkan OK” si no lo está.

Parser: tokens, comillas simples/dobles. **No** declarar pipes/`&&` hasta implementarlos.

---

## 8. UI — DIRECCIÓN VISUAL (MOCKUP)

Referencia mental del mockup del usuario:
- Fondo oscuro, acentos cyan/azul neón
- Sidebar de navegación (Inicio, Red, Terminal, Archivos, Procesos, Sistema, …)
- Panel central: monitor / viewport 3D
- Terminal integrada
- Paneles SYSTEM (CPU, RAM, storage) con datos reales o estado de capacidad
- Top bar: título motor + backend + FPS real
- Herramientas rápidas: visibles pero deshabilitadas/marcada UNAVAILABLE si no existen

**Proyecto 01 UI mínima aceptable:**
- Viewport 3D (o clear GLES + mesh simple)
- HUD: backend, FPS/frames, estado motor
- Terminal usable
- Panel Capabilities
- Navegación básica sin features fantasma

Cada widget debe responder: qué representa, qué dato usa, qué evento, qué acción, qué pasa si falla (§41).

---

## 9. SCENE 3D MÍNIMA (§12–14)

```
Scene
 ├── Camera (orbit / pan / zoom / look-at)
 ├── DirectionalLight
 ├── MeshEntity (vertex/index buffer real)
 └── GroundEntity
```

Prohibido: imagen plana fingiendo 3D, video, CSS, canvas HTML.

Picking: NDC → ray → intersección; BVH preferido (O(N) solo temporal).

---

## 10. LUA (§21–22)

Sandbox por defecto: filesystem/network/process/native = denied.  
Bindings solo APIs públicas: `cyber.system`, `cyber.files`, `cyber.events`, `cyber.scene`, `cyber.ui`, `cyber.shell`, …  
Sin JNI, sin punteros raw, sin registries internos.

---

## 11. DEFINICIÓN DE “IMPLEMENTED” (§51)

Solo si: compila, arranca, operación real, datos reales, maneja errores, tiene test, acceptance criterion, ownership, threading, CapabilityManager, **sin implementación falsa detrás de la interfaz**.

Si no existe: `NOT_IMPLEMENTED` — no “COMING SOON” animado con datos fake.

---

## 12. FASES DE IMPLEMENTACIÓN (ORDEN)

| Fase | Contenido | Estado |
|------|-----------|--------|
| 01 Foundation | Core…CapabilityManager, Engine | **DONE** |
| 02 Android | Bridge, surface, lifecycle | **MOSTLY DONE** |
| 03 Renderer | Vulkan + GLES completo | **GLES básico DONE** |
| 04 Scene | Mesh, camera, light, BVH | **TODO** |
| 05 UI | Dashboard estilo mockup | **TODO** |
| 06 Shell | Comandos + terminal UI | **Parcial** |
| 07 Lua | VM + sandbox | **TODO** |
| 08 System | FS / process / network obs. | **TODO** |
| 09 Integration | Vertical slice §50 | **TODO** |

### Vertical slice obligatorio (§50)
```
Android → boot → capabilities → renderer → 3D scene → touch
→ EventBus → UI → Terminal → cyber version
```

---

## 13. CRITERIO DE FINALIZACIÓN P01 (§54) — CHECKLIST

- [x] APK installs/launches (generado; validar en device)
- [x] C++ engine boots
- [x] EventBus works
- [x] CapabilityManager works
- [ ] Vulkan when supported
- [x] OpenGL ES fallback path exists (clear+present)
- [ ] Real 3D mesh renders
- [ ] Camera works
- [ ] Touch works
- [ ] Picking works
- [ ] UI dashboard
- [ ] Terminal in app
- [ ] cyber version / capabilities in app UI
- [ ] diagnostics in app
- [ ] Lua sandbox test script
- [ ] filesystem basic ops
- [x] error states observable (partial)
- [x] pause/resume + surface recreation (implemented in bridge)
- [ ] architecture tests
- [ ] full acceptance tests

---

## 14. ESTRUCTURA DE REPO

```
cyber-engine/
├── engine/          # C++ core (headers + gles sources)
├── android/         # Gradle app, JNI, SurfaceView
├── tests/unit/      # Host tests
├── tools/           # host_shell_demo
├── docs/            # este documento
├── cyber-engine-v2-debug.apk
└── CMakeLists.txt
```

APK prioritario: `cyber-engine-v2-debug.apk` (GLES + frames).

---

## 15. INSTRUCCIONES PARA EL AGENTE RECEPTOR

1. **Leer este documento completo** antes de escribir código.
2. **No reescribir** módulos DONE sin necesidad; extender.
3. **Mantener** CapabilityManager como única fuente de verdad de disponibilidad.
4. **UI primero como layout** está bien; **rellenar datos solo con APIs reales**.
5. **Prioridad técnica siguiente:** Scene 3D mínima + cámara táctil + terminal en Android + panel capabilities.
6. **Si falta NDK/SDK en el entorno:** instalar (cmdline-tools, platform 34, NDK 26, Gradle 8.x) — no abandonar.
7. **Cada PR/cambio** debe poder explicar qué capability demuestra.
8. **Idioma de UI:** español o bilingüe según el mockup del usuario (labels en ES).
9. **No** depender de Termux.
10. **Producto final visual** ≈ mockup; **semántica** = motor real.

---

## 16. REALITY CHECK (OBLIGATORIO POR MÓDULO)

1. ¿Código real?  
2. ¿Operación real?  
3. ¿Datos reales?  
4. ¿Puede fallar de verdad?  
5. ¿Usuario observa resultado?  
6. ¿Error observable?  
7. ¿Test?  
8. ¿Contrato?  
9. ¿Respeta Android?  
10. ¿Limitaciones documentadas?

Si falla uno → no marcar IMPLEMENTED.

---

## 17. PRINCIPIO FINAL

```
REAL ENGINE → REAL DATA → REAL EVENTS → REAL STATE
→ REAL INTERACTION → REAL VISUALIZATION
```

Nunca al revés.  
La estética futurista es capa superior. La realidad operacional es la base.

---

**Fin de la especificación adaptada v1.1**  
Adjuntar junto con el árbol `cyber-engine/` y el mockup de referencia visual del usuario.
