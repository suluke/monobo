use std::fs::File;
use std::path::PathBuf;
use std::io::prelude::*;
use std::io::{self, BufReader};

fn line_to_int(line: &str) -> usize {
    line.parse::<usize>().unwrap()
}

fn part1() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("02.txt");
    let input_file = File::open(input_path)?;
    let input_reader = BufReader::new(input_file);
    struct State {
        depth: usize,
        pos: usize
    }
    let state = State{depth: 0, pos: 0};
    let result = input_reader
    .lines()
    .map(io::Result::<String>::unwrap)
    .scan(state, |state, line| {
        state.depth = line.strip_prefix("up ").map(line_to_int).map_or(state.depth, |dist| { state.depth - dist });
        state.depth = line.strip_prefix("down ").map(line_to_int).map_or(state.depth, |dist| { state.depth + dist });
        state.pos = line.strip_prefix("forward ").map(line_to_int).map_or(state.pos, |dist| { state.pos + dist });
        Some(state.depth * state.pos)
    }).last().unwrap();
    println!("day2/pt1: {}", result);
    Ok(())
}

fn part2() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("02.txt");
    let input_file = File::open(input_path)?;
    let input_reader = BufReader::new(input_file);
    struct State {
        depth: usize,
        pos: usize,
        aim: usize
    }
    let state = State{depth: 0, pos: 0, aim: 0};
    let result = input_reader
    .lines()
    .map(io::Result::<String>::unwrap)
    .scan(state, |state, line| {
        state.aim = line.strip_prefix("up ").map(line_to_int).map_or(state.aim, |dist| { state.aim - dist });
        state.aim = line.strip_prefix("down ").map(line_to_int).map_or(state.aim, |dist| { state.aim + dist });
        state.pos = line.strip_prefix("forward ").map(line_to_int).map_or(state.pos, |dist| { state.pos + dist });
        state.depth = line.strip_prefix("forward ").map(line_to_int).map_or(state.depth, |dist| { state.depth + dist * state.aim });
        Some(state.depth * state.pos)
    }).last().unwrap();
    println!("day2/pt2: {}", result);
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
