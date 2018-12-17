use std::io::*;
use std::thread::*;
use std::io::Write;
use std::io;
use std::sync::*;

struct Vars { a: f64, b: f64 }

#[allow(unused)]
fn get_input(query: &str) -> String {

    let mut buffer = String::new();
    print!("{}", query);
    stdout().flush();
    stdin().read_line(&mut buffer);

    return buffer;

}

fn inv_poly_num(n: i64, q: f64) -> f64 {

    let v: Vars = get_vars(n);
    let a = v.a;
    let b = v.b;

    let i = -b;
    let j = (b * b + 4.0 * a * q).sqrt();
    let k = 2.0 * a;

    let ans1 = (i + j) / k;
    let ans2 = (i - j) / k;

    if ans1 >= ans2 {
        return ans1;
    } else {
        return ans2;
    }

}

fn all_int(nums: &Vec<f64>) -> bool {

    let mut ans = true;

    for n in nums {

        ans &= *n == n.round();

    }

    return ans;

}

fn get_vars(n: i64) -> Vars {

    let nf = n as f64;
    let a = (nf - 2.0) / 2.0;
    let b = (-nf + 4.0) / 2.0;

    let f: Vars = Vars { a, b };
    return f;

}

#[allow(deprecated, unused)]
fn main() {

    // println!("{0}\n{1}", inv_poly_num(3, 36.0), inv_poly_num(4, 36.0));

    let mut nums = Arc::new(Mutex::new(vec![]));

    println!("This program will test polygon-numbers with the number of sides 'n'\nPlease enter a list of n's to test for overlap...");

    loop {

        let input = get_input("Please enter your list (seperated by \',\'): ");
        let split = input.split(",");
        let mut snums: Vec<&str> = split.collect();
        let snums_len = snums.len();

        unsafe {
            let num = snums[snums.len() - 1];
            snums[snums_len - 1] = num.slice_unchecked(0, num.len() - 1);
        }

        for n in &snums {

            match n.parse::<i64>() {

                Err(e) => { println!("Error in number {}: {}\nPlease try again...", n, e); continue; },
                Ok(val) => { let mut ns = nums.lock().unwrap(); ns.push(val); }

            }

        }

        break;

    }

    println!("Starting threads...");
    let num_threads = 4;
    let mut t_counter = Arc::new(Mutex::new(0));

    #[must_use]
    let mut handles = vec![];

    for i in 0..num_threads {

        let nums = Arc::clone(&nums);
        let t_counter = Arc::clone(&t_counter);
        #[must_use]
        let handle = spawn(move || {

            println!("Starting thread {}...", i);

            let mut shapes: Vec<f64> = vec![];
            let mut tc = t_counter.lock().unwrap();
            let tt = *tc;
            *tc += 1;

            let mut counter: i64 = 0;
            let mut cur_index: i64 = tt * 50000 + num_threads * counter * 50000;
            let mut s_index = cur_index;
            loop {

                cur_index += 1;
                let mut shapes: Vec<f64> = vec![];
                let mut ns = nums.lock().unwrap();
                for n in &*ns {

                    shapes.push(inv_poly_num(*n, cur_index as f64));

                }

                if all_int(&shapes) {
                    let f = io::stdout();
                    let mut sf = f.lock();
                    writeln!(&mut sf, "[{}]: Found overlap at: {}", i, cur_index);
                }

                if (cur_index - s_index >= 50000) {
                    counter += 1;
                    cur_index = tt * 50000 + num_threads * counter * 50000;
                    s_index = cur_index;

                    let f = io::stdout();
                    let mut sf = f.lock();

                }

            }
        });

        handles.push(handle);

    }

    for h in handles {
        h.join().unwrap();
    }

}
