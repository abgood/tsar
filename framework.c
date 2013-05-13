#include "tsar.h"

/* 从模块目录里加载模块 */
void load_modules(void) {
    int i;
    char buff[LEN_128] = {0};
    char mod_path[LEN_128] = {0};
    struct module *mod;
    int (*mod_register)(struct module *);

    sprintf(buff, "modules");
    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->lib) {
            sprintf(mod_path, "%s/%s.so", buff, mod->name);
            if (!(mod->lib = dlopen(mod_path, RTLD_NOW|RTLD_GLOBAL)))
                do_debug(LOG_ERR, "load_modules: dlopen module %s err %s\n", mod->name, dlerror());
            else{
                mod_register = dlsym(mod->lib, "mod_register");
                if (dlerror()) {
                    do_debug(LOG_ERR, "load_modules: dlsym module %s err %s\n", mod->name, dlerror());
                    break;
                } else {
                    mod_register(mod);
                    mod->enable = 1;
                    mod->spec = 0;
                    do_debug(LOG_INFO, "load_modules: load new module '%s' to mods\n", mod_path);
                }
            }
        }
    }
}
