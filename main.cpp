#include <iostream>
#include <vector>
#include <thread>
#include <math.h>
#include <sstream>
#include <mutex>
#include <chrono>
#include <gmpxx.h>

#define DEBUG false

using namespace std;

struct vars {
    float a, b;
};

mutex mu;
int threads;
vector<int> nums;
mpz_class* index_start;

void print_sync(const string& msg) {

    lock_guard<mutex> lg(mu);

    cout << msg << endl;

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
    print_sync(msg);

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
                print_sync("[" + to_string(t_id) + "]: advancing to " + cur_index->get_str());
        }

        auto shapes = new vector<mpf_class*>();

        for (int n : nums) {
            shapes->emplace_back(inv_poly_num(n, cur_index)); // Memory leak
        }

        if (all_int(*shapes)) {
            string alert;
            alert += "Found overlap at: ";
            alert += cur_index->get_str();
            print_sync(alert);
        }

        for (mpf_class* f : *shapes) {
            delete f;
        }

        delete shapes;

    }

    delete cur_index;
    delete offset;

}

int main(int argc, const char** argv) {

   int t_num = 2;
    if (argc > 1) {
        string s = argv[1];
        stringstream stream(s);
        stream >> t_num;
    }

    cout << "This program will test polygon-numbers with the number of sides of 'n'" << endl;
    cout << "Please enter a list of n's to test for overlap..." << endl;

    cout << "Please enter your list (seperated by \',\'): ";

    string input_s;
    cin >> input_s;

    index_start = new mpz_class();
    cout << "Please enter starting index 'q' (interpreted as 10^q): ";
    string power_s;
    cin >> power_s;
    int power;
    stringstream stream(power_s);
    stream >> power;
    mpz_ui_pow_ui(index_start->get_mpz_t(), 10, power);

    vector<string> snums = split(input_s, ',');

    for (const string& snum : snums) {

        int num;
        stringstream stream(snum);
        stream >> num;
        nums.emplace_back(num);

    }

    threads = 0;
    vector<thread*> handles;
    int precision = pow(2, ceil(log2(power)) + 1);

    precision = (precision <= 64) ? 128 : precision;
    cout << "Floating-point precision: " << precision << endl;
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
