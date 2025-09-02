// Minimal lvml MicroPython user C module
// Provides: lvml.hello()

#include "py/runtime.h"
#include "py/mphal.h"

static mp_obj_t lvml_hello(void) {
    mp_printf(&mp_plat_print, "hello from lvml\n");
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(lvml_hello_obj, lvml_hello);

static const mp_rom_map_elem_t lvml_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_lvml) },
    { MP_ROM_QSTR(MP_QSTR_hello), MP_ROM_PTR(&lvml_hello_obj) },
};
static MP_DEFINE_CONST_DICT(lvml_module_globals, lvml_module_globals_table);

const mp_obj_module_t lvml_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&lvml_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_lvml, lvml_user_cmodule);
