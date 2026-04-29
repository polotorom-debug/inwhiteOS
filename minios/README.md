# MiniOS - Sistema Operativo Básico estilo Linux 0.01

## Estructura de Carpetas

```
minios/
├── boot/
│   ├── boot.S          # Bootloader en Assembly (carga el kernel)
│   └── boot.img.S     # Ensamblador para crear imagen de floppy
├── kernel/
│   ├── main.c         # Punto de entrada del kernel
│   ├── printk.c       # Funciones de impresión (printf, puts)
│   ├── interrupts.c   # Manejo de interrupciones (IDT)
│   ├── isr.S         # Rutinas de servicio de interrupciones
│   ├── timer.c       # Manejo del temporizador (PIT)
│   ├── process.c     # Sistema de procesos y scheduler
│   └── syscall.c     # Manejo de llamadas al sistema
├── drivers/
│   └── keyboard.c    # Driver de teclado
├── mm/
│   └── kmalloc.c     # Gestor de memoria dinámica
├── fs/
│   └── minix_fs.c    # Sistema de archivos MINIX
├── shell/
│   └── shell.c       # Shell interactivo
├── include/
│   └── minios.h      # Header principal con definiciones
├── Makefile          # Archivo de compilación
├── linker.ld         # Script del linker
├── create_image.py   # Script para crear imagen floppy
└── bochsrc.txt       # Configuración de Bochs
```

## Build Instructions (Linux)

### Requisitos

```bash
# Ubuntu/Debian
sudo apt-get install build-essential nasm binutils gcc-multilib

# Fedora
sudo dnf install gcc nasm binutils

# Arch Linux
sudo pacman -S base-devel nasm
```

### Compilar

```bash
cd minios
make clean
make all
```

Esto genera:
- `boot/boot.bin` - Bootloader compilado
- `kernel/kernel.bin` - Kernel compilado
- `floppy.img` - Imagen de disco floppy

## Ejecutar

### QEMU

```bash
make run
```

O manualmente:
```bash
qemu-system-i386 -fda floppy.img -m 16
```

### Bochs

```bash
make bochs
```

O manualmente:
```bash
bochs -f bochsrc.txt
```

## Comandos Disponibles

- `help` - Mostrar ayuda
- `clear` - Limpiar pantalla
- `ls` - Listar archivos
- `echo <texto>` - Imprimir texto
- `uptime` - Tiempo de actividad
- `meminfo` - Información de memoria
- `ps` - Procesos activos
- `reboot` - Reiniciar sistema
- `cat <archivo>` - Ver contenido de archivo

## Características Implementadas

### Bootloader (boot/boot.S)
- Carga desde floppy disk
- Busca archivo KERNEL.BIN
- Configura modo protegido 386
- Pasa control al kernel

### Kernel (kernel/)
- Inicialización básica
- Manejo de interrupciones (IDT/ISR)
- Timer PIT funcional
- Sistema de appels al sistema

### Drivers (drivers/)
- Teclado PS/2 con buffer

### Memoria (mm/)
- Asignador de memoria tipo slab básico
- Heap del kernel en 0x100000

### Sistema de Archivos (fs/)
- Compatibilidad MINIX 1.0
- Inodos, zonas, bitmap de inodos/zonas

### Shell (shell/)
- Consola interactiva
- buffers de entrada

## Notas Técnicas

- Arquitectura: Intel 80386
- Modo: Real modo -> Protegido 32-bit
- Memoria: Sin paginación (modo real extendido)
- Formato: ELF32 ejecutable
- Punto de entrada: 0x1000

## Problemas Conocidos

1. El bootloader es demostrativo - en real hardware floppy debe adaptarse
2. El sistema de archivos es simulado (no accede a disco real)
3. Solo funciona en emuladores QEMU/Bochs

## Licencia

Código de ejemplo educativo - libre de usar y modificar.