#include <vector>
#include<algorithm>
#include<functional>
#include<thread>
#include<queue>
#include<future>

class JobSystem {
    public:
    JobSystem(int threads) : stop_{false} {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back(worker);
        }
    }
    ~JobSystem() {
        stop_ = true;
        condition_.notify_all();
        while(!workers_.empty())
            workers_.back().join();
            workers_.pop_back();
        }

    }


    void worker() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return !tasks_.empty() || stop_; });
                if (stop_ && tasks_.empty()) return;
                task = std::move(tasks_.front());
                tasks_.pop();
            } // Release lock before executing task
            task();
        }
    }

    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.push(task);
        }
        condition_.notify_one();
    }

    template<typename T>
    std::future<std::invoke_result<T()>> enqueue(T task) {
        std::packaged_task<U()> pt(std::move(task));
        std::future<U> result = pt.get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.push(std::move(pt));
        }
        condition_.notify_one();
        return result;
    }
    
    std::condition_variable condition_;
    std::queue<std::function<void ()>> tasks_;
    std::vector<std::thread> workers_;
    bool stop_;
};

int f(float,char);

struct paquete {
    float a;
    char b;

    int operator()() {
        return f(a,b);
    }
};

float g();

typedef int(*intllamable)();
 intllamable gg();

int main(int, char**) {

    JobSystem js{8};

    auto fut = js.enqueue<float (*)(),float>(g);


    int patata;
    float a=2.0f;
    char b= 'b';

    auto f_con_2_y_b = [&]() { return f(a,b); }

    std::vector<int> vi;
    std::sort(vi.begin(),vi.end(),[](auto ap,auto bp){
        if(!(ap%3) && (bp%3)) return ap;
        if((ap%3) && !(bp%3)) return bp;
        return ap>bp;
         });

 condition_.notify_one();
 condition_.notify_all();

a = 13.0f

JobSystem jobs;

         auto task1 = jobs.enqueue([](){return 1;});
         auto task2 = jobs.enqueue([](){return 1.0f;});

         jobs.enqueue([](int a, float b){return 1.0f;},task1,task2);

         







    f_con_2_y_b();

    return 0;
}