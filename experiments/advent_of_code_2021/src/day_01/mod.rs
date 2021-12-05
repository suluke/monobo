use std::fs::File;
use std::path::PathBuf;
use std::io::prelude::*;
use std::io::{self, BufReader};

fn line_to_int(line: String) -> usize {
    line.parse::<usize>().unwrap()
}

pub fn part1() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("01.txt");
    let input_file = File::open(input_path)?;
    let input_reader = BufReader::new(input_file);
    let init = (usize::MAX, 0);
    let result = input_reader
    .lines()
    .map(io::Result::<String>::unwrap)
    .map(line_to_int)
    .fold(init, |acc, val| {
        let (prev, old_count) = acc;
        let new_count = if val > prev { old_count + 1 } else { old_count };
        (val, new_count)
    });
    println!("day1/pt1: {}", result.1);
    Ok(())
}

pub fn part2() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("01.txt");
    let input_file = File::open(input_path)?;
    let input_reader = BufReader::new(input_file);
    let mut lines = input_reader
    .lines()
    .map(io::Result::<String>::unwrap)
    .map(line_to_int);
    let v1 = lines.next().unwrap();
    let v2 = lines.next().unwrap();
    let v3 = lines.next().unwrap();
    let init = (&mut[v1, v2, v3], 0_usize);
    let result = lines
    .fold(init, |acc, val| {
        let (window, old_count) = acc;
        let old_sum: usize = window.iter().sum();
        window.rotate_right(1);
        window[0] = val;
        let new_sum: usize = window.iter().sum();
        let new_count = if new_sum > old_sum { old_count + 1 } else { old_count };
        (window, new_count)
    });
    println!("day1/pt2: {}", result.1);
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
