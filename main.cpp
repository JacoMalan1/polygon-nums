#include <iostream>
#include <vector>
#include <thread>
#include <math.h>
#include <sstream>
#include <mutex>
#include <chrono>
#include <gmpxx.h>

#define DEBUG false // Debug flag

// Color escape codes
#define COLOR_ERROR "\u001b[31m"
#define COLOR_ENDC "\u001b[0m"
#define COLOR_WARN "\u001b[38:5:202m"
#define COLOR_INFO "\u001b[92m"
#define COLOR_ALERT "\u001b[38:5:198m"

using namespace std;

struct vars {
    float a, b;
};

mutex mu;
int threads;
vector<int> nums;
mpz_class* index_start;

void print_sync(const string& msg, const char* color = "") {

    lock_guard<mutex> lg(mu);

    cout << color << msg << COLOR_ENDC << endl;

}

vars* get_vars(int n) {

    float a = ((float)n - 2.0f) / 2.0f;
    float b = (-(float)n + 4.0f) / 2.0f;

    auto v = new vars();
    v->a = a;
    v->b = b;
    return v;

}

vector<string> split(string input_s, char delim) {

    vector<string> snums = vector<string>();

    unsigned long pos = 0;
    while (pos != -1) {

        pos = input_s.find(delim);
        snums.emplace_back(input_s.substr(0, pos));
        input_s.erase(0, pos + 1);

    }

    return snums;

}

bool all_int(const vector<mpf_class*>& nums) {

    bool res = true;
    for (mpf_class* f : nums) {

        if (*f != trunc(*f)) {
            res = false;
            break;
        }

    }

    return res;

}

mpf_class* inv_poly_num(int n, mpz_class* q) { // mem leak

    vars* v = get_vars(n);
    mpf_class a = v->a;
    mpf_class b = v->b;

    mpf_class i = -b;
    mpf_class j =  sqrt(b * b + 4.0f * a * (*q));
    mpf_class k = 2.0f * a;

    auto ans1 = new mpf_class();
    auto ans2 = new mpf_class();

    *ans1 = (i + j) / k;
    *ans2 = (i - j) / k;

    delete v;

    if (ans1 >= ans2) {
        delete ans2;
        return ans1;
    } else {
        delete ans1;
        return ans2;
    }

}

string get_input(string prompt) {

    string input;
    cout << prompt;
    cin >> input;
    return input;

}

void doWork() {

    int t_id;
    {
        lock_guard<mutex> lock(mu);
        t_id = threads;
        threads++;
        mu.unlock();
    }

    string msg;
    msg += "Thread ";
    msg += to_string(t_id);
    msg += " started...";
    print_sync(msg, COLOR_INFO);

    auto offset = new mpz_class();
    auto cur_index = new mpz_class();

    mpz_ui_pow_ui(offset->get_mpz_t(), 10, 5);
    *offset *= 5;
    *cur_index = (*index_start) + (*offset) * t_id;
    if (DEBUG)
        print_sync("Thread " + to_string(t_id) + " starting at: " + cur_index->get_str());
    while (true) {

        (*cur_index)++;
        if (*cur_index % *offset == 0) {
            *cur_index += threads * (*offset);
            if (DEBUG)
                print_sync("[" + to_string(t_id) + "]: advancing to " + cur_index->get_str(), COLOR_INFO);
        }

        auto shapes = new vector<mpf_class*>();

        for (int n : nums) {
            shapes->emplace_back(inv_poly_num(n, cur_index)); // Memory leak
        }

        if (all_int(*shapes)) {
            string alert;
            alert += "Found overlap at: ";
            alert += cur_index->get_str();
            print_sync(alert, COLOR_ALERT);
        }

        for (mpf_class* f : *shapes) {
            delete f;
        }

        delete shapes;

    }

    delete cur_index;
    delete offset;

}

int to_int(string str) {

    int res;
    stringstream stream(str);
    stream >> res;
    return res;

}

int main(int argc, const char** argv) {

   int t_num = 2;
    if (argc > 1) {
        string s = argv[1];
        t_num = to_int(s);
    }

    print_sync("This program will test polygon-numbers with the number of sides of 'n'", COLOR_INFO);
    print_sync("Please enter a list of n's to test for overlap...", COLOR_INFO);

    string input_s = get_input("Please enter your list (seperated by \',\'): ");

    index_start = new mpz_class();
    string power_s = get_input("Please enter starting index 'q' (interpreted as 10^q): ");

    int power = to_int(power_s);
    mpz_ui_pow_ui(index_start->get_mpz_t(), 10, power);

    vector<string> snums = split(input_s, ',');

    for (const string& snum : snums) {

        int num = to_int(snum);
        nums.emplace_back(num);

    }

    threads = 0;
    vector<thread*> handles;
    int precision = pow(2, ceil(log2(power)) + 1);

    precision = (precision <= 64) ? 128 : precision;
    print_sync("Floating-point precision: " + to_string(precision), COLOR_WARN);
    mpf_set_default_prec(precision);
    this_thread::sleep_for(2s);

    for (int i = 0; i < t_num; i++) {
        auto handle = new thread(doWork);
        handles.emplace_back(handle);
    }

    for (thread* t : handles)
        t->join();

    return 0;

}
