use std::{collections::HashMap, fs::File};

use memmap::MmapOptions;

fn main() -> std::io::Result<()> {
    let mut hashmap = HashMap::<String, (i32, i32, i32, u32)>::new();

    let file = File::open("measurements.txt").unwrap();
    let mmap = unsafe { MmapOptions::new().map(&file).unwrap() };

    let mut i: usize = 0;
    while i < file.metadata().unwrap().len() as usize {
        let j = i;

        while mmap[i as usize] != b';' {
            i += 1;
        }

        let string = unsafe { std::str::from_utf8_unchecked(&mmap[j..i]) };
        if !hashmap.contains_key(string) {
            hashmap.insert(string.to_owned(), (i32::MAX, i32::MIN, 0, 0));
        }

        i += 1;

        let mut j = i;
        while mmap[i as usize] != b'\n' {
            i += 1;
        }

        let mut signum = 1;
        if mmap[j] == b'-' {
            j += 1;
            signum = -1;
        }

        let major: i32 = mmap[j..i - 2]
            .iter()
            .rev()
            .map(|c| *c - b'0')
            .enumerate()
            .fold(0, |acc, (n, m)| acc + m as i32 * 10i32.pow(n as u32));
        let minor = (mmap[i - 1] - b'0') as i32;
        let n = signum * (major * 10 + minor);

        let (mi, ma, sum, count) = hashmap.get_mut(string).unwrap();
        *mi = (*mi).min(n);
        *ma = (*ma).max(n);
        *sum += n;
        *count += 1;

        i += 1;
    }

    for (name, (mi, ma, sum, n)) in hashmap {
        println!("{name};{mi:.2};{ma:.2};{:.2}", sum as f64 / (10. * n as f64));
    }

    Ok(())
}
