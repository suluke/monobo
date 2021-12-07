use std::convert::TryInto;
use std::fs;
use std::path::PathBuf;

struct Board {
    lines: [[Option<usize>; 5]; 5],
}
enum MarkResult {
    NOP,
    WON,
}
impl Board {
    fn mark(&mut self, val: usize) -> MarkResult {
        for line in &mut self.lines {
            for field in line {
                if let Some(cur) = field {
                    if *cur == val {
                        field.take();
                    }
                }
            }
        }
        self.check()
    }
    fn check(&self) -> MarkResult {
        for i in 0..5 {
            if self.lines[i].iter().all(Option::is_none) {
                return MarkResult::WON;
            }
            if self.lines.iter().all(|line| line[i].is_none()) {
                return MarkResult::WON;
            }
        }
        MarkResult::NOP
    }
    fn sum(&self) -> usize {
        self.lines
            .iter()
            .flatten()
            .map(|field| field.unwrap_or(0))
            .sum()
    }
}

struct BoardBuilder {
    boards: Vec<Board>,
    lines: Vec<[Option<usize>; 5]>,
}
impl BoardBuilder {
    fn new() -> Self {
        Self {
            boards: Vec::new(),
            lines: Vec::new(),
        }
    }
    fn push(&mut self, line: [usize; 5]) {
        self.lines.push(line.map(Some));
        if self.lines.len() == 5 {
            let board_raw: [[Option<usize>; 5]; 5] = self.lines[0..].try_into().unwrap();
            self.boards.push(Board { lines: board_raw });
            self.lines.clear();
        }
    }
}

pub fn part1() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("04.txt");
    let input = fs::read_to_string(input_path).unwrap();
    let mut lines = input.lines();
    let nums: Vec<usize> = lines
        .next()
        .unwrap()
        .split(',')
        .map(|num| num.parse::<usize>().unwrap())
        .collect();

    let mut builder = BoardBuilder::new();
    for line in lines {
        if line.len() > 0 {
            let mut line_parsed = [0; 5];
            for i in 0..5 {
                line_parsed[i] = line[i * 3..(i * 3 + 2)].trim().parse::<usize>().unwrap();
            }
            builder.push(line_parsed);
        }
    }
    let mut boards = builder.boards;
    let winner_val = (move || {
        for num in nums {
            for board in &mut boards {
                if let MarkResult::WON = board.mark(num) {
                    return board.sum() * num;
                }
            }
        }
        panic!("No winner")
    })();
    println!("day4/pt1: {}", winner_val);
    Ok(())
}

pub fn part2() -> std::io::Result<()> {
    let mut input_path = PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("04.txt");
    let input = fs::read_to_string(input_path).unwrap();
    let mut lines = input.lines();
    let nums: Vec<usize> = lines
        .next()
        .unwrap()
        .split(',')
        .map(|num| num.parse::<usize>().unwrap())
        .collect();

    let mut builder = BoardBuilder::new();
    for line in lines {
        if line.len() > 0 {
            let mut line_parsed = [0; 5];
            for i in 0..5 {
                line_parsed[i] = line[i * 3..(i * 3 + 2)].trim().parse::<usize>().unwrap();
            }
            builder.push(line_parsed);
        }
    }
    let mut boards = builder.boards;
    let winner_val = (move || {
        for num in nums {
            if boards.len() > 1 {
                boards = boards.drain(0..).filter_map(|mut board|{
                    if matches!(board.mark(num), MarkResult::NOP) {
                        Some(board)
                    } else {
                        None
                    }
                }).collect();
            } else {
                for board in &mut boards {
                    if let MarkResult::WON = board.mark(num) {
                        return board.sum() * num;
                    }
                }
            }
        }
        panic!("No winner")
    })();
    println!("day4/pt2: {}", winner_val);
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
