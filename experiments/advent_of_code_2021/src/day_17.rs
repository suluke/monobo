use std::ops::RangeInclusive;

use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

struct Probe {
    pos: (isize, isize),
    v: (isize, isize),
}
impl Probe {
    fn new(v: (isize, isize)) -> Self {
        Self { pos: (0, 0), v }
    }
    fn step(&mut self) {
        self.pos.0 += self.v.0;
        self.pos.1 += self.v.1;
        self.v.0 += if self.v.0 > 0 {
            -1
        } else if self.v.0 < 0 {
            1
        } else {
            0
        };
        self.v.1 -= 1;
    }
    fn simulate(&mut self, target: &Target) -> bool {
        loop {
            if self.is_inbounds(target) {
                return true;
            }
            if self.is_unreachable(target) || self.is_overshoot(target) {
                return false;
            }
            self.step();
        }
    }
    fn is_inbounds(&self, target: &Target) -> bool {
        target.x.contains(&self.pos.0) && target.y.contains(&self.pos.1)
    }
    fn is_unreachable(&self, target: &Target) -> bool {
        self.pos.0 + (self.v.0 * (self.v.0 + 1)) / 2 < *target.x.start()
    }
    fn is_overshoot(&self, target: &Target) -> bool {
        self.pos.0 > *target.x.end() || self.pos.1 < *target.y.start()
    }
    fn count_possibilities(target: &Target) -> usize {
        let mut count = 0;
        for vy in (*target.y.start())..(-*target.y.start()) {
            for vx in 0..=(*target.x.end()) {
                if Probe::new((vx, vy)).simulate(&target) {
                    count += 1;
                }
            }
        }
        count
    }
}

struct Target {
    x: RangeInclusive<isize>,
    y: RangeInclusive<isize>,
}
impl Target {
    fn parse(input: &str) -> Self {
        let (x, y) = input
            .trim()
            .strip_prefix("target area: ")
            .unwrap()
            .split_once(", ")
            .unwrap();
        let (x0, x1) = x.strip_prefix("x=").unwrap().split_once("..").unwrap();
        let (y0, y1) = y.strip_prefix("y=").unwrap().split_once("..").unwrap();
        let (x0, x1): (isize, isize) = (x0.parse().unwrap(), x1.parse().unwrap());
        let (y0, y1): (isize, isize) = (y0.parse().unwrap(), y1.parse().unwrap());
        Self {
            x: x0..=x1,
            y: y0..=y1,
        }
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let res: isize = (1..(-Target::parse(&input).y.start())).sum();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let target = Target::parse(&input);
    let res = Probe::count_possibilities(&target);
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
        let input = "target area: x=20..30, y=-10..-5";
        let target = Target::parse(input);
        assert_eq!(Probe::count_possibilities(&target), 112);
    }
}
