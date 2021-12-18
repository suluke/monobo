use std::{fmt::Debug, iter::Peekable};

use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

enum NumPart {
    Digit(u8),
    Number(Box<Num>),
}
impl Debug for NumPart {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Digit(d) => write!(f, "{}", *d),
            Self::Number(num) => write!(f, "{:?}", *num),
        }
    }
}
impl NumPart {
    fn from_chars<Chars: Iterator<Item = char>>(input: &mut Peekable<Chars>) -> Self {
        if let Some('[') = input.peek() {
            Self::wrap(Num::from_chars(input))
        } else {
            let mut digits = String::new();
            while let Some(&c) = input.peek() {
                if c.is_digit(10) {
                    input.next();
                    digits.push(c);
                } else {
                    break;
                }
            }
            Self::Digit(digits.parse::<u8>().unwrap())
        }
    }
    fn magnitude(&self) -> usize {
        match self {
            NumPart::Digit(d) => *d as usize,
            NumPart::Number(n) => n.magnitude(),
        }
    }
    fn try_split(self) -> Result<Self, Self> {
        match self {
            NumPart::Digit(d) => {
                if d >= 10 {
                    Ok(Self::wrap(Num(
                        NumPart::Digit(d / 2),
                        NumPart::Digit((d + 1) / 2),
                    )))
                } else {
                    Err(NumPart::Digit(d))
                }
            }
            NumPart::Number(n) => match n.try_split() {
                Ok(n) => Ok(Self::wrap(n)),
                Err(n) => Err(Self::wrap(n)),
            },
        }
    }
    fn exploded(self) -> Result<(u8, u8), Self> {
        match self {
            NumPart::Digit(d) => Err(Self::Digit(d)),
            NumPart::Number(n) => n.exploded().map_err(Self::wrap),
        }
    }
    fn push_left(self, val: u8) -> Result<Self, Self> {
        match self {
            NumPart::Digit(d) => Ok(NumPart::Digit(d + val)),
            NumPart::Number(n) => n.push_left(val).map(Self::wrap).map_err(Self::wrap),
        }
    }
    fn push_right(self, val: u8) -> Result<Self, Self> {
        match self {
            NumPart::Digit(d) => Ok(NumPart::Digit(d + val)),
            NumPart::Number(n) => n.push_right(val).map(Self::wrap).map_err(Self::wrap),
        }
    }
    fn pull_exploded(self, depth: usize) -> Result<(Self, Option<u8>, Option<u8>), Self> {
        match self {
            NumPart::Digit(d) => Err(NumPart::Digit(d)),
            NumPart::Number(n) => match n.pull_exploded(depth) {
                Ok((new, l, r)) => Ok((Self::wrap(new), l, r)),
                Err(old) => Err(Self::wrap(old)),
            },
        }
    }
    fn wrap(n: Num) -> Self {
        Self::Number(Box::new(n))
    }
}
struct Num(NumPart, NumPart);
impl Num {
    fn parse(input: &str) -> Self {
        Self::from_chars(&mut input.chars().peekable())
    }
    fn from_chars<Chars: Iterator<Item = char>>(input: &mut Peekable<Chars>) -> Self {
        let open = input.next().unwrap();
        assert_eq!(open, '[');
        let l = NumPart::from_chars(input);
        let comma = input.next().unwrap();
        assert_eq!(comma, ',');
        let r = NumPart::from_chars(input);
        let close = input.next().unwrap();
        assert_eq!(close, ']');
        Self(l, r)
    }
    fn exploded(self) -> Result<(u8, u8), Self> {
        match self.0 {
            NumPart::Digit(l) => match self.1 {
                NumPart::Digit(r) => Ok((l, r)),
                NumPart::Number(n) => Err(Self(NumPart::Digit(l), NumPart::Number(n))),
            },
            NumPart::Number(n) => Err(Self(NumPart::Number(n), self.1)),
        }
    }
    fn pull_exploded(mut self, depth: usize) -> Result<(Self, Option<u8>, Option<u8>), Self> {
        if depth >= 2 {
            match self.0.exploded() {
                Ok((l, r)) => {
                    let new_l = NumPart::Digit(0);
                    match self.1.push_left(r) {
                        Ok(new_r) => return Ok((Self(new_l, new_r), Some(l), None)),
                        Err(old_r) => return Ok((Self(new_l, old_r), Some(l), Some(r))),
                    }
                }
                Err(old_l) => self.0 = old_l,
            }
            match self.1.exploded() {
                Ok((l, r)) => {
                    let new_r = NumPart::Digit(0);
                    match self.0.push_right(l) {
                        Ok(new_l) => return Ok((Self(new_l, new_r), None, Some(r))),
                        Err(old_l) => return Ok((Self(old_l, new_r), Some(l), Some(r))),
                    }
                }
                Err(old_r) => self.1 = old_r,
            }
        }
        // TODO
        match self.0.pull_exploded(depth + 1) {
            Ok((new_l, l, mut r)) => {
                if let Some(r_val) = r {
                    match self.1.push_left(r_val) {
                        Ok(new_r) => return Ok((Self(new_l, new_r), l, None)),
                        Err(old_r) => {
                            r = Some(r_val);
                            self.1 = old_r;
                        }
                    }
                }
                return Ok((Self(new_l, self.1), l, r));
            }
            Err(old_l) => self.0 = old_l,
        };
        match self.1.pull_exploded(depth + 1) {
            Ok((new_r, mut l, r)) => {
                if let Some(l_val) = l {
                    match self.0.push_right(l_val) {
                        Ok(new_l) => return Ok((Self(new_l, new_r), None, r)),
                        Err(old_l) => {
                            l = Some(l_val);
                            self.0 = old_l;
                        }
                    }
                }
                return Ok((Self(self.0, new_r), l, r));
            }
            Err(old_r) => self.1 = old_r,
        };
        Err(self)
    }
    fn push_left(self, val: u8) -> Result<Self, Self> {
        match self.0.push_left(val) {
            Ok(new) => Ok(Self(new, self.1)),
            Err(old_l) => match self.1.push_left(val) {
                Ok(new_r) => Ok(Self(old_l, new_r)),
                Err(old_r) => Err(Self(old_l, old_r)),
            },
        }
    }
    fn push_right(self, val: u8) -> Result<Self, Self> {
        match self.1.push_right(val) {
            Ok(new) => Ok(Self(self.0, new)),
            Err(old_r) => match self.0.push_right(val) {
                Ok(new_l) => Ok(Self(new_l, old_r)),
                Err(old_l) => Err(Self(old_l, old_r)),
            },
        }
    }
    fn try_explode(mut self) -> Result<Self, Self> {
        match self.0.pull_exploded(0) {
            Ok((new_l, _, rval)) => {
                if let Some(rval) = rval {
                    self.1 = self.1.push_left(rval).unwrap();
                }
                Ok(Self(new_l, self.1))
            }
            Err(mut old_l) => match self.1.pull_exploded(0) {
                Ok((new_r, lval, _)) => {
                    if let Some(lval) = lval {
                        old_l = old_l.push_right(lval).unwrap();
                    }
                    Ok(Self(old_l, new_r))
                }
                Err(old_r) => Err(Self(old_l, old_r)),
            },
        }
    }
    fn try_split(self) -> Result<Self, Self> {
        match self.0.try_split() {
            Ok(l) => return Ok(Self(l, self.1)),
            Err(l) => match self.1.try_split() {
                Ok(r) => Ok(Self(l, r)),
                Err(r) => Err(Self(l, r)),
            },
        }
    }
    fn reduce(self) -> Self {
        let mut res = self;
        // println!("Unreduced: {:?}", res);
        loop {
            match res.try_explode() {
                Ok(ex) => {
                    res = ex;
                    // println!("After explode: {:?}", res);
                }
                Err(unex) => match unex.try_split() {
                    Ok(split) => {
                        res = split;
                        // println!("After split: {:?}", res);
                    }
                    Err(unsplit) => return unsplit,
                },
            }
        }
    }
    fn magnitude(&self) -> usize {
        3 * self.0.magnitude() + 2 * self.1.magnitude()
    }
    fn add(self, other: Num) -> Self {
        // println!("  {:?}", self);
        // println!("+ {:?}", other);
        let res = Self(NumPart::wrap(self), NumPart::wrap(other)).reduce();
        // println!("= {:?}\n", res);
        res
    }
}
impl Debug for Num {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "[{:?},{:?}]", self.0, self.1)
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let res = input
        .lines()
        .map(Num::parse)
        .reduce(Num::add)
        .unwrap()
        .magnitude();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let mut max = 0;
    for l1 in input.lines() {
        for l2 in input.lines() {
            if l1 == l2 {
                continue;
            }
            let l1 = Num::parse(l1);
            let l2 = Num::parse(l2);
            // let eq = format!("  {:?}\n+ {:?}", l1, l2);
            let sum = l1.add(l2);
            // let res = format!("= {:?}", sum);
            let mag = sum.magnitude();
            if mag > max {
                // println!("{}\n{}\n=>{}", eq, res, mag);
                max = mag;
            }
        }
    }
    println!("day{}/pt2: {}", day(), max);
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
        assert_eq!(Num::parse("[9,1]").magnitude(), 29);
        assert_eq!(
            Num::parse("[[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]]").magnitude(),
            3488
        );
        assert_eq!(
            format!("{:?}", Num::parse("[2,4]").push_left(3).unwrap()),
            "[5,4]"
        );
        assert_eq!(
            format!("{:?}", Num::parse("[2,4]").push_right(3).unwrap()),
            "[2,7]"
        );
        assert_eq!(&format!("{:?}", Num::parse("[15,0]").reduce()), "[[7,8],0]");
        assert_eq!(
            Num::parse("[15,0]").reduce().magnitude(),
            3 * (3 * 7 + 2 * 8)
        );
        let l1 = "[1,1]\n\
        [2,2]\n\
        [3,3]\n\
        [4,4]";
        let n1 = l1.lines().map(Num::parse).reduce(Num::add).unwrap();
        assert_eq!(format!("{:?}", n1), "[[[[1,1],[2,2]],[3,3]],[4,4]]");

        let s = "[[[[4,3],4],4],[7,[[8,4],9]]]\n[1,1]";
        let ns = s.lines().map(Num::parse).reduce(Num::add).unwrap();
        assert_eq!(format!("{:?}", ns), "[[[[0,7],4],[[7,8],[6,0]]],[8,1]]");

        let d = "[[[0,[4,5]],[0,0]],[[[4,5],[2,6]],[9,5]]]\n\
        [7,[[[3,7],[4,3]],[[6,3],[8,8]]]]\n\
        [[2,[[0,8],[3,4]]],[[[6,7],1],[7,[1,6]]]]\n\
        [[[[2,4],7],[6,[0,5]]],[[[6,8],[2,8]],[[2,1],[4,5]]]]\n\
        [7,[5,[[3,8],[1,4]]]]\n\
        [[2,[2,2]],[8,[8,1]]]\n\
        [2,9]\n\
        [1,[[[9,3],9],[[9,0],[0,7]]]]\n\
        [[[5,[7,4]],7],1]\n\
        [[[[4,2],2],6],[8,7]]";
        let nd = d.lines().map(Num::parse).reduce(Num::add).unwrap();
        assert_eq!(
            format!("{:?}", nd),
            "[[[[8,7],[7,7]],[[8,6],[7,7]]],[[[0,7],[6,6]],[8,7]]]"
        );

        let h = "[[[0,[5,8]],[[1,7],[9,6]]],[[4,[1,2]],[[1,4],2]]]\n\
        [[[5,[2,8]],4],[5,[[9,9],0]]]\n\
        [6,[[[6,2],[5,6]],[[7,6],[4,7]]]]\n\
        [[[6,[0,7]],[0,9]],[4,[9,[9,0]]]]\n\
        [[[7,[6,4]],[3,[1,3]]],[[[5,5],1],9]]\n\
        [[6,[[7,3],[3,2]]],[[[3,8],[5,7]],4]]\n\
        [[[[5,4],[7,7]],8],[[8,3],8]]\n\
        [[9,3],[[9,9],[6,[4,9]]]]\n\
        [[2,[[7,7],7]],[[5,8],[[9,3],[0,2]]]]\n\
        [[[[5,2],5],[8,[3,7]]],[[5,[7,5]],[4,4]]]";
        let nh = h.lines().map(Num::parse).reduce(Num::add).unwrap();
        assert_eq!(
            format!("{:?}", nh),
            "[[[[6,6],[7,6]],[[7,7],[7,0]]],[[[7,7],[7,7]],[[7,8],[9,9]]]]"
        );
    }
}
