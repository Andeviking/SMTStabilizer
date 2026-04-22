#include <ctime>
#include <ios>
#include <iostream>

#include "kernel/kernel.h"
#include "node/node_manager.h"
#include "parser/parser.h"

int main(int argc, char *argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    stabilizer::parser::Parser p;
    p.getOptions()->setKeepLet(false);
    p.getOptions()->setExpandFunctions(false);
    if (argc == 2)
        p.parse(argv[1]);
    else
        p.parse("");
    // p.parse("/pub/netdisk1/zhangx/benchmark-2025/non-incremental/MARIPOSA/unstable_ext/fs_dice-queries-ASN1.Spec.Value.INTEGER-25.smt2");

    stabilizer::node::NodeManager nm(p);
    nm.simplify_assertions();
    // stabilizer::rewrite::Rewriter rewriter(nm);

    // rewriter.apply();

    stabilizer::kernel::Kernel kernel(nm);
    struct timespec t_start, t_end;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_start))
        perror("clock gettime");

    kernel.apply(nm);

    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_end))
        perror("clock gettime");
    double time_used = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec) / 1e9;
    // std::cout << std::fixed << std::setprecision(2) << time_used << std::endl;
    std::cout << nm.to_string() << std::endl;
    return 0;
}
