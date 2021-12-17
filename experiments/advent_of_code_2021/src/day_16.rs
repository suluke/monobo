use std::ops::{Add, Mul};

use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

#[derive(Clone)]
struct BitGen<'a> {
    stream: &'a str,
    pos: u8,
}
impl<'a> BitGen<'a> {
    fn new(stream: &'a str) -> Self {
        Self { stream, pos: 0 }
    }
}
impl<'a> Iterator for BitGen<'a> {
    type Item = bool;

    fn next(&mut self) -> Option<Self::Item> {
        if self.pos >= 4 {
            self.pos = 0;
            self.stream = &self.stream[1..];
        }
        if self.stream.is_empty() {
            return None;
        }
        let c = self.stream.chars().nth(0).unwrap() as u8;
        let val = if c >= '0' as u8 && c <= '9' as u8 {
            c - '0' as u8
        } else if c >= 'a' as u8 && c <= 'f' as u8 {
            c - 'a' as u8 + 10
        } else if c >= 'A' as u8 && c <= 'F' as u8 {
            c - 'A' as u8 + 10
        } else {
            panic!("Not a hex char: {}", c);
        };
        let res = ((val >> (3 - self.pos)) & 1) != 0;
        self.pos += 1;
        Some(res)
    }
}

fn push_bit<T: Mul<Output = T> + Add<Output = T> + From<u8>>(acc: T, b: bool) -> T {
    acc * 2.into() + if b { 1.into() } else { 0.into() }
}

enum Content {
    Literal(u64),
    Operator(Vec<Packet>),
}
impl Content {
    fn literal_from_bits<T: Iterator<Item = bool>>(bits: &mut T) -> Self {
        let mut acc: u64 = 0;
        loop {
            let cont = bits.next().unwrap();
            acc = acc << 4;
            acc |= bits.take(4).fold(0, push_bit);
            if !cont {
                break;
            }
        }
        Self::Literal(acc)
    }
    fn operator_from_bits<T: Iterator<Item = bool>>(bits: &mut T) -> Self {
        if bits.next().unwrap() {
            let num_subpkgs = bits.take(11).fold(0, push_bit);
            let mut subpkgs = Vec::with_capacity(num_subpkgs);
            for _ in 0..num_subpkgs {
                subpkgs.push(Packet::from_bits(bits));
            }
            Self::Operator(subpkgs)
        } else {
            let num_bits: usize = bits.take(15).fold(0, push_bit);
            // must move the bits out, otherwise we have an endless recursion of Take<Peekable<Take<... as iterator type
            let bits_collected: Vec<bool> = bits.take(num_bits).collect();
            let mut iter = bits_collected.iter().copied().peekable();
            let mut subpkgs = Vec::new();
            while iter.peek().is_some() {
                subpkgs.push(Packet::from_bits(&mut iter));
            }
            Self::Operator(subpkgs)
        }
    }
}
struct Packet {
    ver: u8,
    ty: u8,
    content: Content,
}
impl Packet {
    fn parse(input: &str) -> Self {
        let mut bits = BitGen::new(input.trim());
        Self::from_bits(&mut bits)
    }
    fn from_bits<T: Iterator<Item = bool>>(bits: &mut T) -> Self {
        let ver = bits.take(3).fold(0, push_bit);
        let ty = bits.take(3).fold(0, push_bit);
        let content = if ty == 4 {
            Content::literal_from_bits(bits)
        } else {
            Content::operator_from_bits(bits)
        };
        Self { ver, ty, content }
    }

    fn version_sum(&self) -> usize {
        if let Content::Operator(subpkgs) = &self.content {
            self.ver as usize + subpkgs.iter().map(Self::version_sum).sum::<usize>()
        } else {
            self.ver as usize
        }
    }
    fn eval(&self) -> u64 {
        match &self.content {
            Content::Literal(val) => *val,
            Content::Operator(operands) => {
                if self.ty == 0 {
                    operands.iter().map(Self::eval).sum()
                } else if self.ty == 1 {
                    operands.iter().map(Self::eval).product()
                } else if self.ty == 2 {
                    operands.iter().map(Self::eval).min().unwrap()
                } else if self.ty == 3 {
                    operands.iter().map(Self::eval).max().unwrap()
                } else if self.ty == 5 {
                    if operands[0].eval() > operands[1].eval() {
                        1
                    } else {
                        0
                    }
                } else if self.ty == 6 {
                    if operands[0].eval() < operands[1].eval() {
                        1
                    } else {
                        0
                    }
                } else if self.ty == 7 {
                    if operands[0].eval() == operands[1].eval() {
                        1
                    } else {
                        0
                    }
                } else {
                    panic!("Unknown operation type {}", self.ty);
                }
            }
        }
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let res = Packet::parse(&input).version_sum();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let res = Packet::parse(&input).eval();
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
        assert_eq!(
            BitGen::new("D2FE28").fold(String::new(), |acc, b| acc + if b { "1" } else { "0" }),
            "110100101111111000101000"
        );
        assert_eq!(Packet::parse("8A004A801A8002F478").version_sum(), 16);
        assert_eq!(
            Packet::parse("620080001611562C8802118E34").version_sum(),
            12
        );
        assert_eq!(
            Packet::parse("C0015000016115A2E0802F182340").version_sum(),
            23
        );
        assert_eq!(
            Packet::parse("A0016C880162017C3686B18A3D4780").version_sum(),
            31
        );
    }
    #[test]
    fn test2() {
        assert_eq!(Packet::parse("C200B40A82").eval(), 3);
        assert_eq!(Packet::parse("04005AC33890").eval(), 54);
        assert_eq!(Packet::parse("880086C3E88112").eval(), 7);
        assert_eq!(Packet::parse("CE00C43D881120").eval(), 9);
        assert_eq!(Packet::parse("D8005AC2A8F0").eval(), 1);
        assert_eq!(Packet::parse("F600BC2D8F").eval(), 0);
        assert_eq!(Packet::parse("9C005AC2F8F0").eval(), 0);
        assert_eq!(Packet::parse("9C0141080250320F1802104A08").eval(), 1);
    }
}
