#!/bin/bash
# Tab 补全脚本 — source 后 make <Tab> 会自动列出 test_* 目标
# 用法: source m_test/make_completion.sh
# 或加入 ~/.bashrc: source /home/mi/Mys/prj/rcmw/m_test/make_completion.sh

_make_test_targets() {
    local cur prev
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    # 只在 make 后面第一个参数时触发补全
    if [[ ${COMP_CWORD} -eq 1 ]]; then
        # 在 m_test 目录下运行 make list，获取所有目标名
        local targets
        if [[ -f Makefile ]] && grep -q '^list:' Makefile; then
            targets=$(make -s list 2>/dev/null)
        fi
        # 补全：加上 help / all / clean / cleanobj / list
        targets="$targets help all clean cleanobj list"
        COMPREPLY=( $(compgen -W "$targets" -- "$cur") )
    fi
}

complete -F _make_test_targets make
