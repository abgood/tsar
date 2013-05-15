#include "tsar.h"

/* 从模块目录里加载模块 */
void load_modules(void) {
    int i;
    char buff[LEN_128] = {0};
    char mod_path[LEN_128] = {0};
    struct module *mod;
    int (*mod_register)(struct module *);

    sprintf(buff, "/root/code/tsar/modules");
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

/* 注册模块区域,完善模块相关信息 */
void register_mod_fileds(struct module *mod, const char *opt, const char *usage,
        struct mod_info *info, int n_col, void *data_collect, void *set_st_record) {
    sprintf(mod->opt_line, "%s", opt);
    sprintf(mod->usage, "%s", usage);
    mod->info = info;
    mod->n_col = n_col;
    mod->data_collect = data_collect;
    mod->set_st_record = set_st_record;
}

/* 设置module收集到的信息到mod->record */
void set_mod_record(struct module *mod, const char *record) {
    if (record)
        sprintf(mod->record, "%s", record);
}

/* tsar运行开始先收集数据再输出 */
void collect_record(void) {
    int i;
    struct module *mod = NULL;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (!mod->enable)
            continue;

        memset(mod->record, 0, sizeof(mod->record));
        if (mod->data_collect)
            mod->data_collect(mod, mod->parameter);
    }
}

/* 循环第一个字串, 是否包含第二个字串, 有则返回1 */
int is_include_string(const char *mods, const char *mod) {
    char *token, n_str[LEN_512] = {0};

    /* 第一字串拷到n_str里 */
    memcpy(n_str, mods, strlen(mods));

    /* 第一字串里的每一项都要与第二字串比较 */
    token = strtok(n_str, DATA_SPLIT);
    while (token) {
        if (!strcmp(token, mod))
            return 1;
        token = strtok(NULL, DATA_SPLIT);
    }
    return 0;
}

/* 重新加载输出设置模块
 *  如果不在mods里, disable此模块
 */
int reload_modules(const char *s_mod) {
    int i;
    int reload = 0;
    struct module *mod;

    if (!s_mod || !strlen(s_mod))
        return reload;

    for (i = 0; i < statis.total_mod_num; i++) {
        mod = &mods[i];
        if (is_include_string(s_mod, mod->name) || is_include_string(s_mod, mod->opt_line)) {
            mod->enable = 1;
            reload = 1;
        } else
            mod->enable = 0;
    }
    return reload;
}
