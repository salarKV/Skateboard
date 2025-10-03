[document.txt](https://github.com/user-attachments/files/22682994/document.txt)
I've never played skateboarding games before, so I spent the first few hours gathering and playing some of them. After trying a few and reading community comments, I got to Tony Hawk's Pro Skater 4. It's an old game, but I enjoyed playing it—it felt smooth and satisfying—and I based some of my next decisions on that experience.

I went with using a pawn and physics-based movement instead of character movement to have a nice simulation. I used set linear physics and angular physics instead of adding force and torque to have good control over exact numbers for the pawn. After basic forward movement, stopping, and turning left and right, I slowly started fine-tuning it, like having the skate rotation reset after falling or adding turn-in-place rotation.

After that, I added a ramp. I locked the board on a plane (based on the ramp's forward vector) after taking off, added some fake gravity based on the board's velocity once it was in the air, allowed for rotation in the air, and made it so that no matter which side of the board landed, it would still land properly. After fine-tuning the ramp, I added a simple jump and an event dispatcher to keep track of the amount of rotation in the air, giving points to the player based on that. That was the end of my time.

There were still things I would have loved to polish, like auto-correction once the board hits a block, handling different landing scenarios, or adding the character skeletal mesh.

Overall, this was a great experience and I loved it. If it's approved so far, I’d gladly go back and complete those additions. I can’t say confidently how long each task took, but the most time-consuming one was the ramp. Overall, the project took 12 hours of active development and experimenting.
