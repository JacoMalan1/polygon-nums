#include <iostream>
#include <vector>
#include <thread>
#include <math.h>
#include <sstream>
#include <mutex>
#include <gmpxx.h>

#define DEBUG false

using namespace std;

struct vars {
    mpf_class a, b;

};

mutex mu;
int threads;
vector<int> nums;

vars* get_vars(const mpz_class& n) {

    mpf_class a = ((mpf_class)n - 2.0f) / 2.0f;
    mpf_class b = (-(mpf_class)n + 4.0f) / 2.0f;

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

        if (*f != ceil(*f)) {
            res = false;
            break;
        }

    }

    return res;

}

mpf_class* inv_poly_num(const mpz_class& n, const mpf_class& q) { // mem leak

    vars* v = get_vars(n);
    mpf_class a = v->a;
    mpf_class b = v->b;

    mpf_class i = -b;
    mpf_class j =  sqrt(b * b + 4.0f * a * q);
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

void print_sync(const string& msg) {

    lock_guard<mutex> lg(mu);

    cout << msg << endl;

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

    mpz_class offset = 5 * pow(10, 5);
    mpz_class cur_index = offset * t_id;
    if (DEBUG)
        print_sync("Thread " + to_string(t_id) + " starting at: " + cur_index.get_str());
    while (true) {

        cur_index++;
        if (cur_index % offset == 0) {
            cur_index += threads * offset;
            if (DEBUG)
                print_sync("[" + to_string(t_id) + "]: advancing to " + cur_index.get_str());
        }

        auto shapes = new vector<mpf_class*>();

        for (int n : nums) {
            shapes->emplace_back(inv_poly_num(n, cur_index)); // Memory leak
        }

        if (all_int(*shapes)) {
            string alert;
            alert += "Found overlap at: ";
            alert += cur_index.get_str();
            print_sync(alert);
        }

        for (mpf_class* f : *shapes) {
            delete f;
        }

        delete shapes;

    }

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

    vector<string> snums = split(input_s, ',');

    for (const string& snum : snums) {

        int num;
        stringstream stream(snum);
        stream >> num;
        nums.emplace_back(num);

    }

    threads = 0;
    vector<thread*> handles;
    for (int i = 0; i < t_num; i++) {
        auto handle = new thread(doWork);
        handles.emplace_back(handle);
    }

    for (thread* t : handles)
        t->join();

    return 0;

}
