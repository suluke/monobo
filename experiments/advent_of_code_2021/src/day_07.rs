fn part1() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("07.txt");
    let input = std::fs::read_to_string(input_path).unwrap();
    let positions: Vec<usize> = input
        .trim()
        .split(",")
        .map(|val| val.parse::<usize>().unwrap())
        .collect();
    let min = positions.iter().copied().min().unwrap();
    let max = positions.iter().copied().max().unwrap();
    let mut min_dist = usize::MAX;
    for checked in min..=max {
        min_dist = usize::min(
            min_dist,
            positions
                .iter()
                .copied()
                .map(|pos| {
                    if pos < checked {
                        checked - pos
                    } else {
                        pos - checked
                    }
                })
                .sum(),
        );
    }
    println!("day7/pt1: {}", min_dist);
    Ok(())
}

fn part2() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("07.txt");
    let input = std::fs::read_to_string(input_path).unwrap();
    let positions: Vec<usize> = input
        .trim()
        .split(",")
        .map(|val| val.parse::<usize>().unwrap())
        .collect();
    let min = positions.iter().copied().min().unwrap();
    let max = positions.iter().copied().max().unwrap();
    let mut min_dist = usize::MAX;
    for checked in min..=max {
        min_dist = usize::min(
            min_dist,
            positions
                .iter()
                .copied()
                .map(|pos| {
                    let dist = if pos < checked {
                        checked - pos
                    } else {
                        pos - checked
                    };
                    (dist * (dist+1)) / 2
                })
                .sum(),
        );
    }
    println!("day7/pt2: {}", min_dist);
    Ok(())
}

pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
