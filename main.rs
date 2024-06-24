use std::{
    collections::HashMap, f32::{INFINITY, NEG_INFINITY}, fs::File, io::{BufRead, BufReader}
};

fn main() {
    let mut hashmap = HashMap::<String, (f32, f32, f32, u32)>::new();

    let file = File::open("measurements.txt").unwrap();

    let reader = BufReader::new(file);

    for line in reader.lines() {
        let line = line.unwrap();
        let (left, right) = line.split_once(';').unwrap();
        let (mi, ma, sum, n) = hashmap.entry(left.to_string()).or_insert((INFINITY, NEG_INFINITY, 0., 0));
        
        let x = right.parse::<f32>().unwrap();
        *mi = mi.min(x);
        *ma = ma.max(x);
        *sum += x;
        *n += 1;
    }

    for (name, (mi, ma, sum, n)) in hashmap {
        println!("{name};{mi:.2};{ma:.2};{:.2}", sum / n as f32);
    }
}
