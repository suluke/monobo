use std::fs;
use std::path::PathBuf;

fn part1() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("03.txt");

    struct State {
        one_counts: Vec<usize>,
        total: usize,
    }
    let state = State {
        one_counts: Vec::with_capacity(12),
        total: 0,
    };
    let state = fs::read_to_string(input_path)
        .unwrap()
        .lines()
        .fold(state, |mut state, line| {
            state.total += 1;
            for (idx, a_char) in line.chars().enumerate() {
                if state.one_counts.len() <= idx {
                    state.one_counts.push(0);
                }
                state.one_counts[idx] += if a_char == '0' { 0 } else { 1 };
            }
            state
        });
    let gamma = state
        .one_counts
        .iter()
        .map(|val| -> usize { (*val > (state.total / 2)) as usize })
        .fold(0, |acc, val| acc * 2 + val);
    let eps = state
        .one_counts
        .iter()
        .map(|val| -> usize { (*val < (state.total / 2)) as usize })
        .fold(0, |acc, val| acc * 2 + val);
    println!("day3/pt1: {}", gamma * eps);
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()
}
