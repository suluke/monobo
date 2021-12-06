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

fn oxgen(lines: &[String]) -> usize {
    let mut rem = lines.to_vec();
    let mut pos = 0;
    while rem.len() > 1 {
        let mc = if rem.iter().fold(0, |acc, val| {
            acc + (val.chars().nth(pos).unwrap() == '1') as usize
        }) * 2 >= rem.len() {'1'} else {'0'};
        rem = rem.iter().filter(|str| {str.chars().nth(pos).unwrap() == mc}).cloned().collect();
        pos += 1;
    }
    rem[0]
        .chars()
        .fold(0, |acc, bit| acc * 2 + (bit == '1') as usize)
}
fn coscrub(lines: &[String]) -> usize {
    let mut rem = lines.to_vec();
    let mut pos = 0;
    while rem.len() > 1 {
        let mc = if rem.iter().fold(0, |acc, val| {
            acc + (val.chars().nth(pos).unwrap() == '0') as usize
        }) * 2 <= rem.len() {'0'} else {'1'};
        rem = rem.iter().filter(|str| {str.chars().nth(pos).unwrap() == mc}).cloned().collect();
        pos += 1;
    }
    rem[0]
        .chars()
        .fold(0, |acc, bit| acc * 2 + (bit == '1') as usize)
}

fn part2() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("03.txt");
    let lines: Vec<String> = fs::read_to_string(input_path)
        .unwrap()
        .lines()
        .map(str::to_owned)
        .collect();
    let ox = oxgen(&lines);
    let co = coscrub(&lines);
    println!("day3/pt2: {}", ox * co);
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn pt2() {
        let lines: Vec<String> =
            "00100\n11110\n10110\n10111\n10101\n01111\n00111\n11100\n10000\n11001\n00010\n01010"
                .lines()
                .map(str::to_owned)
                .collect();
        let ox = oxgen(&lines);
        let co = coscrub(&lines);
        assert_eq!(ox, 23);
        assert_eq!(co, 10);
    }
}
