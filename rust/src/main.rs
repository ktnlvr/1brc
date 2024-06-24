use fxhash::FxHashMap;
use std::fs::File;

use memmap::MmapOptions;

fn main() -> std::io::Result<()> {
    let mut hashmap = FxHashMap::<String, (i32, i32, i32, u32)>::default();

    let file = File::open("measurements.txt").unwrap();
    let mmap = unsafe { MmapOptions::new().map(&file).unwrap() };

    let mut i: usize = 0;
    let len = file.metadata().unwrap().len() as usize;

    while i < len {
        let j = i;

        while mmap[i] != b';' {
            i += 1;
        }

        let string = unsafe { std::str::from_utf8_unchecked(&mmap[j..i]) };
        if !hashmap.contains_key(string) {
            hashmap.insert(string.to_owned(), (i32::MAX, i32::MIN, 0, 0));
        }

        i += 1;

        let mut j = i;
        while mmap[i] != b'\n' {
            i += 1;
        }

        let mut signum = 1;
        if mmap[j] == b'-' {
            j += 1;
            signum = -1;
        }

        let mut major = 0;
        let iters = i - 2 - j;
        for k in 0..iters {
            const POWERS_OF_TEN: [i32; 4] = [1, 10, 100, 1000];
            let d = mmap[j + k] - b'0';
            major += d as i32 * POWERS_OF_TEN[iters - k];
        }

        let minor = (mmap[i - 1] - b'0') as i32;
        let n = signum * (major + minor);

        let (mi, ma, sum, count) = hashmap.get_mut(string).unwrap();
        *mi = (*mi).min(n);
        *ma = (*ma).max(n);
        *sum += n;
        *count += 1;

        i += 1;
    }

    for (name, (mi, ma, sum, n)) in hashmap {
        println!(
            "{name};{mi:.2};{ma:.2};{:.1}",
            sum as f64 / (10. * n as f64)
        );
    }

    Ok(())
}
