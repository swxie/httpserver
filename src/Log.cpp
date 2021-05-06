#include "Log.h"

string Log::log_file_name;
queue<string> Log::log_queue; 
mutex Log::log_queue_mutex;
string Log::log_path;
bool Log::info_on = true;
bool Log::debug_on = true;
bool Log::warn_on = true;
bool Log::error_on = true;
sem_t Log::log_queue_sem;
bool Log::stop_status = false;
bool Log::finished_status = false;

void Log::
init(string path, bool Info_on, bool Debug_on, bool Warn_on, bool Error_on){
    log_path = path;
    info_on = Info_on;
    debug_on = Debug_on;
    warn_on = Warn_on;
    error_on = Error_on;
    sem_init(&log_queue_sem, 0, 0);
    Time t;
    log_file_name = log_path + t.getDate() + ".log";
    thread write_thread(Log::asyncWriteFile);
    write_thread.detach();
}

void Log::log(string info, Level level){
    string log_string;
    Time t;
    string time_s = t.getTime();
    log_string += (time_s + " ");
    switch(level){
        case INFO:{
            if(!info_on) return;
            log_string += "[INFO]:";
            //cout<< time_s << " \033[32m[INFO]: \033[0m" << info << endl;
            break;
        }
        case DEBUG:{
            if(!debug_on) return;
            log_string += "[DEBUG]:";
            //cout << time_s << " \033[34m[DEBUG]: \033[0m" << info << endl;
            break;
        }
        case WARN:{
            if(!warn_on) return;
            log_string += "[WARN]:";
            //cout << time_s << " \033[33m[WARN]: \033[0m" << info << endl;
            break;
        }
        case ERROR:{
            if(!error_on) return;
            log_string += "[ERROR]:";
            //cout << time_s << " \033[31m[ERROR]: \033[0m" << info << endl;
            break;
        }
    }

    log_string += info;
    log_queue_mutex.lock();
    log_queue.push(log_string);
    log_queue_mutex.unlock();
    sem_post(&log_queue_sem);
}

void Log::stop(){
    stop_status = true;
}

void Log::asyncWriteFile(){
    ofstream out_log(log_file_name,ios::app);
    if(out_log.fail()){
        Log::log("async log write thread start failed",ERROR);
        throw runtime_error("async log write thread start failed");
    }
    stop_status = false;
    while(!stop_status){
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;
        if (sem_timedwait(&log_queue_sem, &ts) == 0){
            log_queue_mutex.lock();
            string log_string = log_queue.front();
            log_queue.pop();
            log_queue_mutex.unlock();
            out_log << log_string << endl;
        }
    }
    while (!log_queue.empty()){
        string log_string = log_queue.front();
        out_log << log_string << endl;
        log_queue.pop();
    }
    out_log.close();
    finished_status = true;
}

bool Log::finished(){
    return finished_status;
}

Time::Time(){
    time_t t = time(0);
    char ch[64];
    strftime(ch, sizeof(ch), "%Y-%m-%d %H:%M:%S", localtime(&t)); //年-月-日 时-分-秒
    time_str = ch;
}

string Time::getDate(){
    size_t pos = time_str.find(" ");
    return time_str.substr(0,pos);
}

string Time::getTime(){
    size_t pos = time_str.find(" ");
    return time_str.substr(pos + 1);
}