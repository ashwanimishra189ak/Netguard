#include "load_balancer.h"
#include <iostream>
#include <utility>

namespace DPI {

// ============================================================
// LoadBalancer
// ============================================================

LoadBalancer::LoadBalancer(
    int lb_id,
    std::vector<ThreadSafeQueue<PacketJob>*> fp_queues,
    int fp_start_id)
    : lb_id_(lb_id),
      fp_start_id_(fp_start_id),
      num_fps_(static_cast<int>(fp_queues.size())),
      input_queue_(10000),
      fp_queues_(std::move(fp_queues)) {

    // IMPORTANT:
    // Prevent the original segmentation fault:
    // per_fp_counts_ was previously size 0.
    per_fp_counts_.resize(num_fps_, 0);

    std::cout << "[LB" << lb_id_ << "] Created with "
              << num_fps_ << " FPs\n";
}

LoadBalancer::~LoadBalancer() {
    stop();
}

void LoadBalancer::start() {
    if (running_) {
        return;
    }

    running_ = true;
    thread_ = std::thread(&LoadBalancer::run, this);

    std::cout << "[LB" << lb_id_
              << "] Started (serving FP"
              << fp_start_id_ << "-FP"
              << (fp_start_id_ + num_fps_ - 1)
              << ")\n";
}

void LoadBalancer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    input_queue_.shutdown();

    if (thread_.joinable()) {
        thread_.join();
    }

    std::cout << "[LB" << lb_id_ << "] Stopped\n";
}

void LoadBalancer::run() {

    while (running_) {

        // Your ThreadSafeQueue::pop() takes NO timeout argument.
        auto job_opt = input_queue_.pop();

        if (!job_opt) {
            break;
        }

        PacketJob job = std::move(*job_opt);

        packets_received_++;

        int fp_index = selectFP(job.tuple);

        if (fp_index < 0 || fp_index >= num_fps_) {
            std::cerr << "[LB" << lb_id_
                      << "] Invalid FP index: "
                      << fp_index << "\n";
            continue;
        }

        // Send packet to selected FP.
        fp_queues_[fp_index]->push(std::move(job));

        packets_dispatched_++;

        per_fp_counts_[fp_index]++;
    }
}

int LoadBalancer::selectFP(const FiveTuple& tuple) {

    if (num_fps_ <= 0) {
        return -1;
    }

    FiveTupleHash hasher;

    size_t hash_value = hasher(tuple);

    return static_cast<int>(hash_value %
                            static_cast<size_t>(num_fps_));
}

LoadBalancer::LBStats LoadBalancer::getStats() const {

    LBStats stats{};

    stats.packets_received =
        packets_received_.load();

    stats.packets_dispatched =
        packets_dispatched_.load();

    stats.per_fp_packets =
        per_fp_counts_;

    return stats;
}


// ============================================================
// LBManager
// ============================================================

LBManager::LBManager(
    int num_lbs,
    int fps_per_lb,
    std::vector<ThreadSafeQueue<PacketJob>*> fp_queues)
    : fps_per_lb_(fps_per_lb) {

    for (int lb = 0; lb < num_lbs; ++lb) {

        std::vector<ThreadSafeQueue<PacketJob>*> lb_fp_queues;

        int start = lb * fps_per_lb_;

        for (int i = 0; i < fps_per_lb_; ++i) {

            int fp_index = start + i;

            if (fp_index >= 0 &&
                fp_index < static_cast<int>(fp_queues.size())) {

                lb_fp_queues.push_back(fp_queues[fp_index]);
            }
        }

        lbs_.push_back(
            std::make_unique<LoadBalancer>(
                lb,
                std::move(lb_fp_queues),
                start
            )
        );
    }

    std::cout << "[LBManager] Created "
              << lbs_.size()
              << " load balancers, "
              << fps_per_lb_
              << " FPs each\n";
}

LBManager::~LBManager() {
    stopAll();
}

void LBManager::startAll() {

    for (auto& lb : lbs_) {
        lb->start();
    }
}

void LBManager::stopAll() {

    for (auto& lb : lbs_) {
        lb->stop();
    }
}

LoadBalancer& LBManager::getLBForPacket(
    const FiveTuple& tuple) {

    FiveTupleHash hasher;

    size_t index =
        hasher(tuple) % lbs_.size();

    return *lbs_[index];
}

LBManager::AggregatedStats
LBManager::getAggregatedStats() const {

    AggregatedStats result{};

    result.total_received = 0;
    result.total_dispatched = 0;

    for (const auto& lb : lbs_) {

        LoadBalancer::LBStats stats =
            lb->getStats();

        result.total_received +=
            stats.packets_received;

        result.total_dispatched +=
            stats.packets_dispatched;
    }

    return result;
}

} // namespace DPI
