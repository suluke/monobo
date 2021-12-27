use std::{convert::TryInto, fmt::Debug};

use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

#[derive(Copy, Clone)]
enum Axis3 {
    X = 0,
    Y = 1,
    Z = 2,
}

trait Rot90: Clone {
    fn rot90(&self, pivot: Axis3) -> Self;
    fn iter_rot90(&self) -> IterRot90<Self> {
        IterRot90::new(self)
    }
}

#[derive(Clone, PartialEq, Eq, PartialOrd, Ord)]
struct Point3 {
    coords: [i16; 3],
}
impl Point3 {
    fn new(coords: &[i16; 3]) -> Self {
        Self {
            coords: coords.clone(),
        }
    }
    #[allow(unused)]
    fn xyz(x: i16, y: i16, z: i16) -> Self {
        Self { coords: [x, y, z] }
    }
}
impl Rot90 for Point3 {
    fn rot90(&self, pivot: Axis3) -> Self {
        let mut coords = self.coords.clone();
        coords.rotate_left(pivot as usize);
        coords.swap(1, 2);
        coords[1] *= -1;
        coords.rotate_right(pivot as usize);
        Self { coords }
    }
}
impl std::ops::Add for &Point3 {
    type Output = Point3;

    fn add(self, rhs: Self) -> Self::Output {
        Point3 {
            coords: [
                self.coords[0] + rhs.coords[0],
                self.coords[1] + rhs.coords[1],
                self.coords[2] + rhs.coords[2],
            ],
        }
    }
}
impl std::ops::Sub for &Point3 {
    type Output = Point3;

    fn sub(self, rhs: Self) -> Self::Output {
        Point3 {
            coords: [
                self.coords[0] - rhs.coords[0],
                self.coords[1] - rhs.coords[1],
                self.coords[2] - rhs.coords[2],
            ],
        }
    }
}
impl Debug for Point3 {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(
            f,
            "({}, {}, {})",
            self.coords[0], self.coords[1], self.coords[2]
        )
    }
}
impl From<&[i16; 3]> for Point3 {
    fn from(coords: &[i16; 3]) -> Self {
        Self::new(coords)
    }
}

#[derive(Clone, Debug)]
struct Scan {
    points: Vec<Point3>,
}

impl Scan {
    fn from_lines<'a, T: Iterator<Item = &'a str>>(lines: &mut T) -> Option<Self> {
        if let Some(header) = lines.next() {
            assert!(header.starts_with("--- "));
        } else {
            return None;
        }
        let mut points = Vec::new();
        loop {
            let p_line = lines.next();
            match p_line {
                Some(p_line) => {
                    if p_line.len() < 3 {
                        break;
                    } else {
                        let p: [i16; 3] = p_line
                            .split(",")
                            .map(|s| s.parse::<i16>().unwrap())
                            .collect::<Vec<_>>()
                            .try_into()
                            .unwrap();
                        points.push(Point3::new(&p));
                    }
                }
                None => break,
            }
        }
        Some(Self { points })
    }
    fn is_match(&self, other: &Self) -> bool {
        let mut matches = 0;
        for p in &self.points {
            matches += if other.points.contains(p) { 1 } else { 0 };
        }
        matches >= 12
    }
    fn translate(&self, t: &Point3) -> Self {
        Self {
            points: self.points.iter().map(|p| p + t).collect(),
        }
    }
    fn try_match(&self, ref_scan: &Scan) -> Option<(Scan, Point3)> {
        for p in &self.points {
            for o in &ref_scan.points {
                let t = o - p;
                let translated = self.translate(&t);
                if translated.is_match(ref_scan) {
                    println!("Matched with t={:?}", t);
                    return Some((translated, t));
                }
            }
        }
        None
    }
    fn try_rotate_match(&self, ref_scan: &Scan) -> Option<(Scan, usize, Point3)> {
        self.iter_rot90()
            .enumerate()
            .find_map(|(idx, rotation)| rotation.try_match(ref_scan).map(|(s, t)| (s, idx, t)))
    }
}
impl Rot90 for Scan {
    fn rot90(&self, pivot: Axis3) -> Self {
        Self {
            points: self.points.iter().map(|p| p.rot90(pivot)).collect(),
        }
    }
}

struct IterRot90<T: Rot90> {
    val: T,
    rot_dir: u8,
    rot_top: u8,
}
impl<T: Rot90> IterRot90<T> {
    fn new(init: &T) -> Self {
        Self {
            val: init.clone(),
            rot_dir: 0,
            rot_top: 0,
        }
    }
}
impl<T: Rot90> Iterator for IterRot90<T> {
    type Item = T;

    fn next(&mut self) -> Option<Self::Item> {
        if self.rot_dir == 6 {
            return None;
        }
        let res = self.val.clone();
        self.val = self.val.rot90(Axis3::X);
        self.rot_top += 1;
        if self.rot_top == 4 {
            self.rot_top = 0;
            self.val = self
                .val
                .rot90(([Axis3::Y, Axis3::Z, Axis3::Y])[self.rot_dir as usize % 3]);
            self.rot_dir += 1;
        }
        Some(res)
    }
}
struct Match {
    #[allow(unused)]
    base: Scan,
    matched: Scan,
    #[allow(unused)]
    rotations: u8,
    translation: Point3,
}

struct Scans {
    scans: Vec<Scan>,
}
impl Scans {
    fn parse(input: &str) -> Self {
        let mut lines = input.lines();
        let mut scans = Vec::new();
        while let Some(scan) = Scan::from_lines(&mut lines) {
            scans.push(scan);
        }
        Self { scans }
    }
    fn match_all(&self) -> Vec<Match> {
        let mut unmatched_scans: Vec<(&Scan, usize)> = self.scans[1..].iter().zip(1..).collect();
        let mut matched_scans: Vec<(&Scan, Scan, usize, Point3, usize)> = vec![(
            &self.scans[0],
            self.scans[0].clone(),
            0,
            Point3::xyz(0, 0, 0),
            0,
        )];
        let mut num_processed = 0;
        while unmatched_scans.len() > 0 {
            let matched_before = matched_scans.len();
            let unmatched_before = unmatched_scans.len();
            unmatched_scans = unmatched_scans
                .drain(0..)
                .filter(|&(unmatched, idx)| {
                    if let Some((s, n, t)) = matched_scans[num_processed..matched_before]
                        .iter()
                        .find_map(|(_base_scan, ref_scan, _ref_n, _ref_t, _)| {
                            // println!("Try matching scan #{} with #{}", idx, ref_idx);
                            if let Some((s, n, t)) = unmatched.try_rotate_match(ref_scan) {
                                // println!("Success");
                                Some((s, n, t))
                            } else {
                                None
                            }
                        })
                    {
                        matched_scans.push((unmatched, s, n, t, idx));
                        false
                    } else {
                        true
                    }
                })
                .collect();
            num_processed = matched_before;
            println!("Remaining: {}", unmatched_scans.len());
            if unmatched_before == unmatched_scans.len() {
                panic!(
                    "No progress.\nMatched:\n{:?}\n==========\nUnmatched:\n{:?}",
                    matched_scans, unmatched_scans
                );
            }
        }
        matched_scans
            .drain(0..)
            .map(|(base, matched, rotations, translation, _)| Match {
                base: base.clone(),
                matched,
                rotations: rotations as u8,
                translation,
            })
            .collect()
    }
    fn count_distinct_points(&self) -> usize {
        self.match_all()
            .iter()
            .map(|m| &m.matched.points)
            .flatten()
            .collect::<std::collections::BTreeSet<_>>()
            .len()
    }
    fn largest_manhattan_distance(&self) -> usize {
        let positions : Vec<_> = self.match_all()
            .iter()
            .map(|m| &m.translation).cloned().collect();
        let mut max = 0;
        for i in 0..positions.len() {
            for j in i..positions.len() {
                let diff = &positions[j] - &positions[i];
                let dist = diff.coords[0].abs() as usize + diff.coords[1].abs() as usize + diff.coords[2].abs() as usize;
                max = max.max(dist);
            }
        }
        max
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let res = Scans::parse(&input).count_distinct_points();
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let res = Scans::parse(&input).largest_manhattan_distance();
    println!("day{}/pt2: {}", day(), res);
    Ok(())
}
pub fn solve() -> std::io::Result<()> {
    part1()?;
    part2()
}
#[cfg(test)]
mod tests {
    use std::collections::BTreeSet;

    use super::*;

    #[test]
    fn test() {
        assert_eq!(Point3::xyz(1, 2, 3).rot90(Axis3::X), (&[1, -3, 2]).into());
        assert_eq!(
            Point3::xyz(1, 2, 3)
                .iter_rot90()
                .collect::<BTreeSet<_>>()
                .len(),
            24
        );
        assert!(Point3::xyz(1, 2, 3)
            .iter_rot90()
            .all(|p| p != Point3::xyz(-1, -2, -3)));
        let ex1 = Scans::parse(
            "--- scanner 0 ---\n\
        -1,-1,1\n\
        -2,-2,2\n\
        -3,-3,3\n\
        -2,-3,1\n\
        5,6,-4\n\
        8,0,7",
        );
        assert_eq!(ex1.scans.len(), 1);
        let ex1 = &ex1.scans[0];
        assert_eq!(ex1.points.len(), 6);
        let ex2 = Scans::parse(
            "--- scanner 0 ---\n\
            404,-588,-901\n\
            528,-643,409\n\
            -838,591,734\n\
            390,-675,-793\n\
            -537,-823,-458\n\
            -485,-357,347\n\
            -345,-311,381\n\
            -661,-816,-575\n\
            -876,649,763\n\
            -618,-824,-621\n\
            553,345,-567\n\
            474,580,667\n\
            -447,-329,318\n\
            -584,868,-557\n\
            544,-627,-890\n\
            564,392,-477\n\
            455,729,728\n\
            -892,524,684\n\
            -689,845,-530\n\
            423,-701,434\n\
            7,-33,-71\n\
            630,319,-379\n\
            443,580,662\n\
            -789,900,-551\n\
            459,-707,401\n\
            \n\
            --- scanner 1 ---\n\
            686,422,578\n\
            605,423,415\n\
            515,917,-361\n\
            -336,658,858\n\
            95,138,22\n\
            -476,619,847\n\
            -340,-569,-846\n\
            567,-361,727\n\
            -460,603,-452\n\
            669,-402,600\n\
            729,430,532\n\
            -500,-761,534\n\
            -322,571,750\n\
            -466,-666,-811\n\
            -429,-592,574\n\
            -355,545,-477\n\
            703,-491,-529\n\
            -328,-685,520\n\
            413,935,-424\n\
            -391,539,-444\n\
            586,-435,557\n\
            -364,-763,-893\n\
            807,-499,-711\n\
            755,-354,-619\n\
            553,889,-390\n\
            \n\
            --- scanner 2 ---\n\
            649,640,665\n\
            682,-795,504\n\
            -784,533,-524\n\
            -644,584,-595\n\
            -588,-843,648\n\
            -30,6,44\n\
            -674,560,763\n\
            500,723,-460\n\
            609,671,-379\n\
            -555,-800,653\n\
            -675,-892,-343\n\
            697,-426,-610\n\
            578,704,681\n\
            493,664,-388\n\
            -671,-858,530\n\
            -667,343,800\n\
            571,-461,-707\n\
            -138,-166,112\n\
            -889,563,-600\n\
            646,-828,498\n\
            640,759,510\n\
            -630,509,768\n\
            -681,-892,-333\n\
            673,-379,-804\n\
            -742,-814,-386\n\
            577,-820,562\n\
            \n\
            --- scanner 3 ---\n\
            -589,542,597\n\
            605,-692,669\n\
            -500,565,-823\n\
            -660,373,557\n\
            -458,-679,-417\n\
            -488,449,543\n\
            -626,468,-788\n\
            338,-750,-386\n\
            528,-832,-391\n\
            562,-778,733\n\
            -938,-730,414\n\
            543,643,-506\n\
            -524,371,-870\n\
            407,773,750\n\
            -104,29,83\n\
            378,-903,-323\n\
            -778,-728,485\n\
            426,699,580\n\
            -438,-605,-362\n\
            -469,-447,-387\n\
            509,732,623\n\
            647,635,-688\n\
            -868,-804,481\n\
            614,-800,639\n\
            595,780,-596\n\
            \n\
            --- scanner 4 ---\n\
            727,592,562\n\
            -293,-554,779\n\
            441,611,-461\n\
            -714,465,-776\n\
            -743,427,-804\n\
            -660,-479,-426\n\
            832,-632,460\n\
            927,-485,-438\n\
            408,393,-506\n\
            466,436,-512\n\
            110,16,151\n\
            -258,-428,682\n\
            -393,719,612\n\
            -211,-452,876\n\
            808,-476,-593\n\
            -575,615,604\n\
            -485,667,467\n\
            -680,325,-822\n\
            -627,-443,-432\n\
            872,-547,-609\n\
            833,512,582\n\
            807,604,487\n\
            839,-516,451\n\
            891,-625,532\n\
            -652,-548,-490\n\
            30,-46,-14",
        );
        let ex2 = ex2.match_all();
        assert_eq!(
            ex2.iter()
                .map(|s| &s.points)
                .flatten()
                .collect::<std::collections::BTreeSet<_>>()
                .len(),
            79
        );
    }
}
