fn part1() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("08.txt");
    let input = std::fs::read_to_string(input_path).unwrap();

    let res = input
        .lines()
        .map(|line| line.split(" | ").nth(1).unwrap())
        .map(|s| s.split(" "))
        .flatten()
        .filter(|&s| [2, 3, 4, 7].contains(&s.len()))
        .count();

    println!("day8/pt1: {}", res);
    Ok(())
}

fn decode_line(line: &str) -> usize {
    let (l, r) = line.trim().split_once(" | ").unwrap();
    let samples: Vec<&str> = l.split(" ").collect();
    let s_1 = samples.iter().copied().find(|&s| s.len() == 2).unwrap();
    let s_4 = samples.iter().copied().find(|&s| s.len() == 4).unwrap();
    let s_7 = samples.iter().copied().find(|&s| s.len() == 3).unwrap();
    let s_8 = samples.iter().copied().find(|&s| s.len() == 7).unwrap();
    let s_9 = samples
    .iter()
    .copied()
    .find(|&s| s.len() == 6 && s_4.chars().all(|c| s.contains(c)))
    .unwrap();
    let s_0 = samples
    .iter()
    .copied()
    .find(|&s| s.len() == 6 && s != s_9 && s_7.chars().all(|c| s.contains(c)))
    .unwrap();
    let s_6 = samples
    .iter()
    .copied()
    .find(|&s| s.len() == 6 && s != s_0 && s != s_9)
    .unwrap();
    let s_5 = samples
    .iter()
    .copied()
    .find(|&s| s.len() == 5 && s.chars().all(|c| s_6.contains(c)))
    .unwrap();
    let s_3 = samples
        .iter()
        .copied()
        .find(|&s| s.len() == 5 && s != s_5 && s.chars().all(|c| s_9.contains(c)))
        .unwrap();
    let s_2 = samples
        .iter()
        .copied()
        .find(|&s| s.len() == 5 && s != s_5 && s != s_3)
        .unwrap();
    let mapping = [s_0, s_1, s_2, s_3, s_4, s_5, s_6, s_7, s_8, s_9];
    r.split(" ").fold(0, |acc, s| {
        let d = mapping
            .iter()
            .position(|&m| s.len() == m.len() && s.chars().all(|c| m.contains(c)))
            .unwrap();
        acc * 10 + d
    })
}

fn part2() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("08.txt");
    let input = std::fs::read_to_string(input_path).unwrap();

    let res: usize = input.lines().map(decode_line).sum();

    println!("day8/pt2: {}", res);
    Ok(())
}
pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}

#[cfg(test)]
mod test {
    use super::*;
    #[test]
    fn test() {
        let input = "acedgfb cdfbe gcdfa fbcad dab cefabd cdfgeb eafb cagedb ab | cdfeb fcadb cdfeb cdbaf";
        assert_eq!(decode_line(input), 5353);
    }
}
