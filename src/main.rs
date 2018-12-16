use std::io::*;

// na : an(n+b) / 2   = (an^2 + abn) / c   a  b     n  b
// 3a : 1n(n+1) / 2   = (n^2 + n) / 2      1  1     3  1
// 4a : 2n(n+0) / 2   = (n^2 + 0n) / 1     2  0     4  0
// 5a : 3n(n-1) / 2   = (3n^2 - 3n) / 2    3 -1     5 -1
// 6a : 4n(n-1/2) / 2 = (2n^2 - n) / 1     4 -15/30   6 -0.5
// 7a : 5n(n-3/5) / 2 = (5n^2 - 3n) / 2    5 -18/30   7 -0.6
// 8a : 6n(n-2/3) / 2 = (3n^2 - 2n) / 1    6 -20/30   8 -0.6*

struct Vars { a: f64, b: f64 }

#[allow(unused)]
fn get_input(query: &str) -> String {

    let mut buffer = String::new();
    print!("{}", query);
    stdout().flush();
    stdin().read_line(&mut buffer);

    return buffer;

}

fn inv_poly_num(n: i32, q: f64) -> f64 {

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

fn search(nums: &Vec<i32>) {

    let mut cur_index: f64 = 5.0;
    loop {

        cur_index += 1.0;
        let mut shapes: Vec<f64> = vec![];
        for n in nums {

            shapes.push(inv_poly_num(*n, cur_index));

        }

        if all_int(&shapes) {
            println!("Found overlap at: {}", cur_index);
        }

    }

}

fn get_vars(n: i32) -> Vars {

    let nf = n as f64;
    let a = (nf - 2.0) / 2.0;
    let b = (-nf + 4.0) / 2.0;

    let f: Vars = Vars { a, b };
    return f;

}

#[allow(deprecated, unused)]
fn main() {

    // println!("{0}\n{1}", inv_poly_num(3, 36.0), inv_poly_num(4, 36.0));

    let mut nums: Vec<i32> = vec![];

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

            match n.parse::<i32>() {

                Err(e) => { println!("Error in number {}: {}\nPlease try again...", n, e); continue; },
                Ok(val) => nums.push(val)

            }

        }

        break;

    }

    println!("Starting thread...");
    let handle = std::thread::spawn(move || search(&nums));
    match handle.join() {
        Err(x) => { println!("Error: {}", stringify!(x)); return; },
        Ok(_) => return
    }

}
