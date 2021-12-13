use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

#[derive(Copy, Clone, Debug)]
enum Axis {
    X,
    Y,
}

fn fold(line: (Axis, usize), points: &mut [(usize, usize)]) {
    let (axis, pos) = line;
    if let Axis::X = axis {
        for p in points {
            if p.0 > pos {
                p.0 = 2 * pos - p.0;
            }
        }
    } else {
        for p in points {
            if p.1 > pos {
                p.1 = 2 * pos - p.1;
            }
        }
    }
}
fn parse(input: &str) -> (Vec<(usize, usize)>, Vec<(Axis, usize)>) {
    let points: Vec<(usize, usize)> = input
        .lines()
        .take_while(|&l| !l.is_empty())
        .map(|l| {
            let (x, y) = l.split_once(",").unwrap();
            (x.parse::<usize>().unwrap(), y.parse::<usize>().unwrap())
        })
        .collect();
    let mut instrs = input.lines();
    for _ in 0..=points.len() {
        instrs.next();
    }
    let instrs: Vec<_> = instrs
        .map(|l| {
            let (axis, n) = l.split_once("=").unwrap();
            let axis = if axis.chars().last().unwrap() == 'x' {
                Axis::X
            } else {
                Axis::Y
            };
            let n: usize = n.parse().unwrap();
            (axis, n)
        })
        .collect();
    (points, instrs)
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let (mut points, instrs) = parse(&input);
    fold(*instrs.first().unwrap(), &mut points);
    let unique: std::collections::HashSet<_> = points.iter().collect();

    let res = unique.len();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let (mut points, instrs) = parse(&input);
    for i in instrs {
        fold(i, &mut points);
    }
    let unique: std::collections::HashSet<_> = points.iter().collect();
    let w = unique.iter().map(|&p| p.0).max().unwrap() + 1;
    let h = unique.iter().map(|&p| p.1).max().unwrap() + 1;

    println!("day{}/pt2:", day());
    for y in 0..h {
        for x in 0..w {
            if unique.contains(&(x, y)) {
                print!("#");
            } else {
                print!(" ");
            }
        }
        println!("");
    }
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
        let input = "6,10\n\
        0,14\n\
        9,10\n\
        0,3\n\
        10,4\n\
        4,11\n\
        6,0\n\
        6,12\n\
        4,1\n\
        0,13\n\
        10,12\n\
        3,4\n\
        3,0\n\
        8,4\n\
        1,10\n\
        2,14\n\
        8,10\n\
        9,0\n\
        \n\
        fold along y=7\n\
        fold along x=5";
        let (mut points, instrs) = parse(&input);
        fold(*instrs.first().unwrap(), &mut points);
        let unique: std::collections::HashSet<_> = points.iter().collect();
        assert_eq!(17, unique.len());
    }
}
