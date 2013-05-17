#include "tsar.h"

/* mod->record输出到nagios */
void output_nagios(void) {
    int now_time;

    /* 当前整分时间 */
    now_time = statis.cur_time - statis.cur_time%60;
    /* 周期为整分 */
    if ((*conf.cycle_time == 0) || (now_time % *conf.cycle_time != 0))
        return;

    /* 非合并模式 */
    conf.print_merge = MERGE_NOT;

    /* 从文件里读记录并保存至st_array */
    if (get_st_array_from_file())
        return;
}
