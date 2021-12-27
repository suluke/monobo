use std::collections::HashMap;

use super::get_input;
fn day() -> &'static str {
    let m = module_path!();
    &m[m.len() - 2..]
}

struct GameState {
    next: usize,
    positions: [u8; 2],
}
impl GameState {
    fn new(positions: &[u8; 2]) -> Self {
        Self {
            next: 0,
            positions: positions.clone(),
        }
    }
    fn apply_roll(&mut self, roll: usize) -> u8 {
        let next_pos = ((self.positions[self.next] as usize + roll) % 10) as u8;
        // println!("Move player {} from {} to {}", self.next + 1, self.positions[self.next] + 1, next_pos + 1);
        self.positions[self.next] = next_pos;
        self.next = (self.next + 1) % 2;
        next_pos
    }
    fn from_input(input: &str) -> Self {
        let positions = input
            .lines()
            .map(|line| {
                let (l, r) = line.trim().split_once(" starting position: ").unwrap();
                let p_idx: usize = l.split_once(" ").unwrap().1.parse::<usize>().unwrap() - 1;
                let p_pos: u8 = r.parse::<u8>().unwrap() - 1;
                (p_idx, p_pos)
            })
            .fold([0; 2], |mut acc, (p_idx, p_pos)| {
                acc[p_idx] = p_pos;
                acc
            });
        Self::new(&positions)
    }
}
struct DeterministicGame<'a> {
    s: &'a mut GameState,
    rolls: usize,
    scores: [usize; 2],
}
impl<'a> DeterministicGame<'a> {
    fn new(state: &'a mut GameState) -> Self {
        Self {
            s: state,
            rolls: 0,
            scores: [0; 2],
        }
    }
    fn roll(&mut self) -> usize {
        let roll = self.rolls % 100 + 1;
        self.rolls += 1;
        // println!("roll {}", roll);
        roll
    }
    fn play(&mut self) {
        let roll = self.roll() + self.roll() + self.roll();
        let player = self.s.next;
        let next_pos = self.s.apply_roll(roll);
        self.scores[player] += next_pos as usize + 1;
    }
}
impl<'a> Iterator for DeterministicGame<'a> {
    type Item = usize;

    fn next(&mut self) -> Option<Self::Item> {
        if self.scores.iter().any(|&score| score >= 1000) {
            None
        } else {
            self.play();
            Some(self.rolls * self.scores[self.s.next])
        }
    }
}

struct DiracGame {
    next: usize,
    wins: [usize; 2],
    // Map position+score to possibilities leading there
    open: [HashMap<(u8, usize), usize>; 2],
    turns: usize,
}
impl DiracGame {
    const ROLL_PROBS: [(u8, usize); 7] = [(3, 1), (4, 3), (5, 6), (6, 7), (7, 6), (8, 3), (9, 1)];

    fn new(start: &GameState) -> Self {
        let mut open_0 = HashMap::new();
        open_0.insert((start.positions[0], 0), 1);
        let mut open_1 = HashMap::new();
        open_1.insert((start.positions[1], 0), 1);
        Self {
            next: 0,
            wins: [0; 2],
            open: [open_0, open_1],
            turns: 0,
        }
    }
    fn play(&mut self) {
        let player = self.next;
        self.turns += 1;
        let mut new_open = HashMap::new();
        for game in &self.open[player] {
            let ((pos, score), freq) = game;
            for (roll, prob) in Self::ROLL_PROBS {
                let new_pos = (*pos + roll) % 10;
                let new_score = score + new_pos as usize + 1;
                let new_freq = freq * prob;
                // println!("Move player {} from {} to {} for score {}", player + 1, pos + 1, new_pos + 1, new_score);
                if new_score < 21 {
                    *new_open.entry((new_pos, new_score)).or_insert(0) += new_freq;
                } else {
                    // println!(
                    //     "Player {} wins another {} times with score {} after {} turns",
                    //     player + 1,
                    //     new_freq,
                    //     new_score,
                    //     self.turns
                    // );
                    self.wins[player] += new_freq * self.open[(player + 1)%2].values().sum::<usize>();
                }
            }
        }
        self.open[self.next] = new_open;
        self.next = (self.next + 1) % 2;
    }
}
impl Iterator for DiracGame {
    type Item = usize;

    fn next(&mut self) -> Option<Self::Item> {
        if self.open[self.next as usize].is_empty() {
            None
        } else {
            self.play();
            Some(self.wins.iter().copied().max().unwrap())
        }
    }
}

fn part1() -> std::io::Result<()> {
    let input = get_input(day());
    let mut state = GameState::from_input(&input);
    let loser_score = DeterministicGame::new(&mut state).last().unwrap();
    let res = loser_score;
    println!("day{}/pt1: {}", day(), res);
    Ok(())
}
fn part2() -> std::io::Result<()> {
    let input = get_input(day());
    let res = DiracGame::new(&GameState::from_input(&input))
        .last()
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
        let input = "Player 1 starting position: 4\n\
                          Player 2 starting position: 8";
        let mut state = GameState::from_input(input);
        let loser_score = DeterministicGame::new(&mut state).last().unwrap();
        let res = loser_score;
        assert_eq!(res, 739785);
        let res = DiracGame::new(&GameState::from_input(&input))
            .last()
            .unwrap();
        assert_eq!(res, 444356092776315);
    }
}
