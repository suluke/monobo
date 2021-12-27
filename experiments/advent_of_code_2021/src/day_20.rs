use std::{collections::BTreeSet, convert::TryInto};

use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

#[derive(Clone)]
struct Algo {
    bits: [bool; 512],
}
impl Algo {
    fn from_line(line: &str) -> Self {
        Self {
            bits: line
                .trim()
                .chars()
                .map(|c| c == '#')
                .collect::<Vec<_>>()
                .try_into()
                .unwrap(),
        }
    }
}
#[derive(Clone)]
struct Image {
    dflt: bool,
    nondflt: BTreeSet<(i32, i32)>,
}
impl Image {
    fn from_lines<'a, I: Iterator<Item = &'a str>>(lines: &mut I) -> Self {
        let mut nondflt = BTreeSet::new();
        for (y, line) in lines.enumerate() {
            for (x, c) in line.trim().chars().enumerate() {
                if c == '#' {
                    nondflt.insert((x as i32, y as i32));
                }
            }
        }
        Self {
            dflt: false,
            nondflt,
        }
    }
    fn neighbor_index(&self, pos: (i32, i32)) -> usize {
        let mut res = 0;
        for y in (pos.1 - 1)..=(pos.1 + 1) {
            for x in (pos.0 - 1)..=(pos.0 + 1) {
                res = (res << 1) + (self.nondflt.contains(&(x, y)) != self.dflt) as usize;
            }
        }
        res
    }

    fn count_lit(&self) -> Option<usize> {
        if !self.dflt {
            Some(self.nondflt.len())
        } else {
            None
        }
    }
}
struct Input(Algo, Image);
impl Input {
    fn parse(input: &str) -> Self {
        let mut lines = input.lines();
        let algo = Algo::from_line(lines.next().unwrap());
        lines.next();
        let img = Image::from_lines(&mut lines);
        Self(algo, img)
    }
}
impl Iterator for Input {
    type Item = Image;

    fn next(&mut self) -> Option<Self::Item> {
        let dflt = self.0.bits[if self.1.dflt { 511 } else { 0 }];
        let mut nondflt = BTreeSet::new();
        for pos in &self.1.nondflt {
            for dy in -1..=1 {
                for dx in -1..=1 {
                    let pos = (pos.0 + dx, pos.1 + dy);
                    if self.0.bits[self.1.neighbor_index(pos)] != dflt {
                        nondflt.insert(pos);
                    }
                }
            }
        }
        self.1 = Image { dflt, nondflt };
        Some(self.1.clone())
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let res = Input::parse(&input)
        .into_iter()
        .nth(1)
        .unwrap()
        .count_lit()
        .unwrap();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let res = Input::parse(&input)
        .into_iter()
        .nth(49)
        .unwrap()
        .count_lit()
        .unwrap();
    println!("day{}/pt2: {}", day(), res);
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
        let img = Image::from_lines(
            &mut "#..#.\n\
                  #....\n\
                  ##..#\n\
                  ..#..\n\
                  ..###"
                .lines(),
        );
        assert_eq!(img.neighbor_index((2, 2)), 34);
        let alg = Algo::from_line(
            "..#.#..#####.#.#.#.###.##.....###.##.#..###.####..#####..#....#..#..##..##\
             #..######.###...####..#..#####..##..#.#####...##.#.#..#.##..#.#......#.###\
             .######.###.####...#.##.##..#..#..#####.....#.#....###..#.##......#.....#.\
             .#..#..##..#...##.######.####.####.#.#...#.......#..#.#.#...####.##.#.....\
             .#..#...##.#.##..#...##.#.##..###.#......#.#.......#.#.#.####.###.##...#..\
             ...####.#..#..#.##.#....##..#.####....##...##..#...#......#.#.......#.....\
             ..##..####..#...#.#.#...##..#.#..###..#####........#..####......#..#",
        );
        let mut state = Input(alg, img);
        let img2 = state.next().unwrap();
        assert_eq!(img2.count_lit().unwrap(), 24);
        let img3 = state.next().unwrap();
        assert_eq!(img3.count_lit().unwrap(), 35);
    }
}
