const OPENERS: [char; 4] = ['(', '[', '{', '<'];
const CLOSERS: [char; 4] = [')', ']', '}', '>'];
const POINTS: [usize; 4] = [3, 57, 1197, 25137];

struct PStack {
    stack: Vec<char>,
}
impl PStack {
    fn new() -> Self {
        Self { stack: Vec::new() }
    }
    fn push(&mut self, c: char) -> std::io::Result<()> {
        if OPENERS.contains(&c) {
            self.stack.push(c);
            return Ok(());
        } else if let Some(pos) = CLOSERS.iter().position(|&d| d == c) {
            if let Some(&top) = self.stack.last() {
                if top == OPENERS[pos] {
                    self.stack.pop();
                    return Ok(());
                }
            }
        }
        use std::io::{Error, ErrorKind};
        Err(Error::new(ErrorKind::Other, format!("{}", c)))
    }
    fn compute_score(&self) -> usize {
        self.stack
            .iter()
            .rev()
            .map(|&c| OPENERS.iter().position(|&d| d == c).unwrap() + 1)
            .fold(0, |acc, val| acc * 5 + val)
    }
}

fn find_illegal(line: &str) -> Option<usize> {
    let illegal_pos = line
        .chars()
        .scan(PStack::new(), |stack, c| stack.push(c).ok())
        .fuse()
        .count();
    if illegal_pos == line.len() {
        None
    } else {
        Some(illegal_pos)
    }
}

fn part1() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("10.txt");
    let input = std::fs::read_to_string(input_path).unwrap();

    let mut res = 0;
    for line in input.lines() {
        if let Some(illegal_pos) = find_illegal(line) {
            let illegal = line.chars().nth(illegal_pos).unwrap();
            let idx = CLOSERS.iter().position(|&c| c == illegal).unwrap();
            res += POINTS[idx];
        }
    }

    println!("day10/pt1: {}", res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("10.txt");
    let input = std::fs::read_to_string(input_path).unwrap();

    let mut scores = Vec::new();
    for line in input.lines().filter(|&l| find_illegal(l).is_none()) {
        let mut stack = PStack::new();
        line.chars().for_each(|c| stack.push(c).unwrap());
        scores.push(stack.compute_score());
    }
    scores.sort();
    let res = scores[scores.len() / 2];

    println!("day10/pt2: {}", res);
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
    fn test() {
        let input = "[({(<(())[]>[[{[]{<()<>>\n\
            [(()[<>])]({[<{<<[]>>(\n\
            {([(<{}[<>[]}>{[]{[(<()>\n\
            (((({<>}<{<{<>}{[]{[]{}\n\
            [[<[([]))<([[{}[[()]]]\n\
            [{[{({}]{}}([{[{{{}}([]\n\
            {<[[]]>}<{[{[{[]{()[[[]\n\
            [<(<(<(<{}))><([]([]()\n\
            <{([([[(<>()){}]>(<<{{\n\
            <{([{{}}[<[[[<>{}]]]>[]]";
        let mut points = 0;
        for line in input.lines() {
            if let Some(illegal_pos) = find_illegal(line) {
                let illegal = line.chars().nth(illegal_pos).unwrap();
                let idx = CLOSERS.iter().position(|&c| c == illegal).unwrap();
                points += POINTS[idx];
            }
        }
        assert_eq!(points, 26397);
    }
    #[test]
    fn test2() {
        let input = "[({(<(())[]>[[{[]{<()<>>";
        let mut stack = PStack::new();
        input.chars().for_each(|c| stack.push(c).unwrap());
        assert_eq!(stack.compute_score(), 288957);
    }
}
