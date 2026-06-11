import sys

content = open('imports.gen.c').read()

replacements = {
    'DUMMY(SL_IID_BUFFERQUEUE)\n': '',
    'DUMMY(SL_IID_EFFECTSEND)\n': '',
    'DUMMY(SL_IID_ENGINE)\n': '',
    'DUMMY(SL_IID_ENGINECAPABILITIES)\n': '',
    'DUMMY(SL_IID_ENVIRONMENTALREVERB)\n': '',
    'DUMMY(SL_IID_PLAY)\n': '',
    'DUMMY(SL_IID_VOLUME)\n': '',
    'DUMMY(__cxa_atexit)\n': '',
    'DUMMY(getauxval)\n': '',
    'DUMMY(slCreateEngine)\n': '',
    'DUMMY(syscall)\n': '',
    'DUMMY(vfprintf)\n': '',
    '{"SL_IID_BUFFERQUEUE", (uintptr_t)&dummy_SL_IID_BUFFERQUEUE}': '{"SL_IID_BUFFERQUEUE", (uintptr_t)&sl_IID_BUFFERQUEUE}',
    '{"SL_IID_EFFECTSEND", (uintptr_t)&dummy_SL_IID_EFFECTSEND}': '{"SL_IID_EFFECTSEND", (uintptr_t)&sl_IID_EFFECTSEND}',
    '{"SL_IID_ENGINE", (uintptr_t)&dummy_SL_IID_ENGINE}': '{"SL_IID_ENGINE", (uintptr_t)&sl_IID_ENGINE}',
    '{"SL_IID_ENGINECAPABILITIES", (uintptr_t)&dummy_SL_IID_ENGINECAPABILITIES}': '{"SL_IID_ENGINECAPABILITIES", (uintptr_t)&sl_IID_ENGINECAPABILITIES}',
    '{"SL_IID_ENVIRONMENTALREVERB", (uintptr_t)&dummy_SL_IID_ENVIRONMENTALREVERB}': '{"SL_IID_ENVIRONMENTALREVERB", (uintptr_t)&sl_IID_ENVIRONMENTALREVERB}',
    '{"SL_IID_PLAY", (uintptr_t)&dummy_SL_IID_PLAY}': '{"SL_IID_PLAY", (uintptr_t)&sl_IID_PLAY}',
    '{"SL_IID_VOLUME", (uintptr_t)&dummy_SL_IID_VOLUME}': '{"SL_IID_VOLUME", (uintptr_t)&sl_IID_VOLUME}',
    '{"__cxa_atexit", (uintptr_t)&dummy___cxa_atexit}': '{"__cxa_atexit", (uintptr_t)&__cxa_atexit}',
    '{"getauxval", (uintptr_t)&dummy_getauxval}': '{"getauxval", (uintptr_t)&getauxval}',
    '{"slCreateEngine", (uintptr_t)&dummy_slCreateEngine}': '{"slCreateEngine", (uintptr_t)&slCreateEngine_shim}',
    '{"syscall", (uintptr_t)&dummy_syscall}': '{"syscall", (uintptr_t)&syscall}',
    '{"vfprintf", (uintptr_t)&dummy_vfprintf}': '{"vfprintf", (uintptr_t)&vfprintf}',
}

for k, v in replacements.items():
    if k not in content:
        print('MISSING:', k)
    content = content.replace(k, v)

if '<sys/auxv.h>' not in content:
    content = '#include <sys/auxv.h>\n' + content

with open('imports.gen.c', 'w') as f:
    f.write(content)
