struct Swarm {
    buckets: [usize; 9],
}
impl Swarm {
    fn load(input: &str) -> Self {
        let mut buckets = [0; 9];
        let fish = input.split(",").map(|val| val.parse::<usize>().unwrap());
        for f in fish {
            buckets[f] += 1;
        }
        Self { buckets }
    }
    fn step(&mut self) {
        let born = self.buckets[0];
        self.buckets.as_mut_slice().rotate_left(1);
        self.buckets[6] += born;
    }
    fn size(&self) -> usize {
        self.buckets.iter().sum()
    }
}

fn part1() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("06.txt");
    let input = std::fs::read_to_string(input_path).unwrap();
    let mut swarm = Swarm::load(input.trim());
    for _ in 0..80 {
        swarm.step();
    }
    println!("day6/pt1: {}", swarm.size());
    Ok(())
}

fn part2() -> std::io::Result<()> {
    let mut input_path = std::path::PathBuf::new();
    input_path.push(env!("CARGO_MANIFEST_DIR"));
    input_path.push("input");
    input_path.push("06.txt");
    let input = std::fs::read_to_string(input_path).unwrap();
    let mut swarm = Swarm::load(input.trim());
    for _ in 0..256 {
        swarm.step();
    }
    println!("day6/pt2: {}", swarm.size());
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
    fn example() {
        let mut swarm = Swarm::load("3,4,3,1,2");
        for _ in 0..18 {
            swarm.step();
        }
        assert_eq!(swarm.size(), 26);
        let mut swarm = Swarm::load("3,4,3,1,2");
        for _ in 0..80 {
            swarm.step();
        }
        assert_eq!(swarm.size(), 5934);
    }
}
