use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

use std::collections::HashMap;

fn parse(input: &str) -> (String, HashMap<(char, char), usize>, HashMap<(char, char), char>) {
    let mut lines = input.lines();
    let tmplt = lines.next().unwrap().trim().to_owned();
    let mut poly = HashMap::new();
    for idx in 1..tmplt.len() {
        let k = (
            tmplt.chars().nth(idx - 1).unwrap(),
            tmplt.chars().nth(idx).unwrap(),
        );
        poly.insert(k, 1);
    }
    let _ = lines.next();
    let rules = lines
        .map(|l| {
            let (k, v) = l.split_once(" -> ").unwrap();
            let k = (k.chars().nth(0).unwrap(), k.chars().nth(1).unwrap());
            (k, v.chars().nth(0).unwrap())
        })
        .collect();
    (tmplt, poly, rules)
}
fn step(
    poly: &HashMap<(char, char), usize>,
    rules: &HashMap<(char, char), char>,
) -> HashMap<(char, char), usize> {
    let mut next = HashMap::<(char, char), usize>::new();
    for (k, v) in poly {
        if let Some(c) = rules.get(k) {
            *next.entry((k.0, *c)).or_insert(0) += *v;
            *next.entry((*c, k.1)).or_insert(0) += *v;
        }
    }
    next
}
fn score(tmplt: &str, poly: &HashMap<(char, char), usize>) -> usize {
    let mut counts = HashMap::<char, usize>::new();
    for ((c1, c2), v) in poly {
        *counts.entry(*c1).or_insert(0) += v;
        *counts.entry(*c2).or_insert(0) += v;
    }
    *counts.entry(tmplt.chars().nth(0).unwrap()).or_insert(0) += 1;
    *counts.entry(tmplt.chars().last().unwrap()).or_insert(0) += 1;
    let min = *counts.values().min().unwrap() / 2;
    let max = *counts.values().max().unwrap() / 2;
    max - min
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let (tmplt, mut poly, rules) = parse(&input);
    for _ in 0..10 {
        poly = step(&poly, &rules);
    }
    let res = score(&tmplt, &poly);
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let (tmplt, mut poly, rules) = parse(&input);
    for _ in 0..40 {
        poly = step(&poly, &rules);
    }
    let res = score(&tmplt, &poly);
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
        let input = "NNCB\n\
        \n\
        CH -> B\n\
        HH -> N\n\
        CB -> H\n\
        NH -> C\n\
        HB -> C\n\
        HC -> B\n\
        HN -> C\n\
        NN -> C\n\
        BH -> H\n\
        NC -> B\n\
        NB -> B\n\
        BN -> B\n\
        BB -> N\n\
        BC -> B\n\
        CC -> N\n\
        CN -> C";
        let (tmplt, mut poly, rules) = parse(&input);
        assert_eq!(1, score(&tmplt, &step(&poly, &rules)));
        for _ in 0..10 {
            poly = step(&poly, &rules);
        }
        assert_eq!(1588, score(&tmplt, &poly));
    }
}
