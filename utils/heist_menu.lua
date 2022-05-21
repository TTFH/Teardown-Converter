-- 1) Make sure this file is NOT called main.lua and add it to a heist map through the editor
-- 2) Set the number of primary, secondary and required targets in the table below
mission = {
	primary = 0,
	secondary = 8,
	required = 4,
}

function init()
	state = ""
	score = 0
	timeLeft = 0

	savedScore = false
	highScore = false

	endTimer = 0
	endScreen = 0
	endScreenScale = 0
	endScreenHeight = 300

	gScoreShown = 0
	gScoreTimer = 0
	gScoreStatsScale = 0
	gScoreCanvasHeight = 0
end

function tick(dt)
	state = GetString("level.state")
	if state == "win" and not savedScore then
		score = GetInt("level.clearedprimary") + GetInt("level.clearedsecondary")
		timeLeft = GetFloat("level.timeleft")

		local oldScore = GetInt("savegame.mod.score")
		local oldTimeLeft = GetFloat("savegame.mod.timeleft")
		if score > oldScore or (score == oldScore and timeLeft > oldTimeLeft) then
			highScore = true
			SetInt("savegame.mod.score", score)
			SetFloat("savegame.mod.timeleft", timeLeft)
		end
		savedScore = true
	end

	if state ~= "" then
		endTimer = endTimer + dt
		if endTimer > 2 then
			if endScreen == 0 then
				SetValue("endScreen", 1, "cosine", 0.25)
			end
		end
	end
end

function draw()
	if endScreen > 0 then
		SetValue("endScreenScale", 1, "easeout", 0.5)
		drawEndScreen(endScreenScale, state)
	end
end

function clamp(value, mi, ma)
	if value < mi then
		value = mi
	end
	if value > ma then
		value = ma
	end
	return value
end

function drawEndScreen(f, state)
	if f > 0 then
		UiPush()
		UiMakeInteractive()
		UiTranslate(-300 + 300 * f, 0)

		UiAlign("top left")
		UiColor(0, 0, 0, 0.7 * f)
		UiRect(400, UiHeight())
		UiWindow(400, UiHeight())
		UiColor(1, 1, 1)
		UiPush()
		UiTranslate(0, 50)

		if state == "win" then
			UiPush()
				UiTranslate(UiCenter(), 0)
				UiAlign("center top")
				UiFont("bold.ttf", 44)
				UiScale(2)
				UiText("MISSION")
				UiTranslate(0, 35)
				UiScale(0.7)
				UiText("COMPLETED")
			UiPop()

			UiPush()
				local primary = GetInt("level.clearedprimary")
				local secondary = GetInt("level.clearedsecondary")
				local timeLeft = GetFloat("level.timeleft")
				local missionTime = GetFloat("level.missiontime")
				local score = primary + secondary

				UiTranslate(UiCenter(), 150)
				UiAlign("center")
				UiFont("bold.ttf", 32)
				if highScore then
					UiText("New highscore " .. score)
				else
					UiText("Score " .. score)
				end

				UiTranslate(-210, 20)
				h = drawScore(score, timeLeft, missionTime)
			UiPop()
			UiTranslate(0, h)
		else
			local h
			UiPush()
				UiTranslate(UiCenter(), 0)
				UiAlign("center top")

				UiPush()
					UiFont("bold.ttf", 44)
					UiScale(2)
					UiColor(0.8, 0, 0)
					UiText("MISSION")
					UiTranslate(0, 32)
					UiScale(1.27)
					UiColor(1, 0, 0)
					UiText("FAILED")
				UiPop()

				UiFont("regular.ttf", 22)
				UiAlign("top left")
				UiTranslate(-144, 180)
				UiColor(0.8, 0.8, 0.8)
				UiWordWrap(290)

				local reason = ""
				if state == "fail_dead" then
					reason = "You died. Explosions, fire, falling and bullets can hurt you. Keep an eye on the health meter"
				elseif state == "fail_alarmtimer" then
					reason = "You failed to escape before security arrived. Make sure to plan properly."
				elseif state == "fail_missiontimer" then
					reason = "You ran out of time. Try again and find better shortcuts."
				end
				_, h = UiText(reason)
			UiPop()
			UiTranslate(0, 40 + h)
		end
		UiPop()
		UiTranslate(0, UiHeight() - endScreenHeight)

		UiPush()
		UiTranslate(UiCenter(), 0)
		UiFont("regular.ttf", 26)
		UiAlign("center middle")
		UiButtonImageBox("ui/common/box-outline-6.png", 6, 6, 1, 1, 1, 0.8)

		if state == "win" then
			UiPush()
				UiTranslate(0, -20)
				UiColor(0.7, 1, 0.8, 0.2)
				UiImageBox("ui/common/box-solid-6.png", 260, 40, 6, 6)
				UiColor(1, 1, 1)
				if UiTextButton("Continue", 260, 40) then
					Menu()
				end
			UiPop()
			UiTranslate(0, 47)
		end

		UiPush()
			if not GetBool("game.canquickload") then
				UiDisableInput()
				UiColorFilter(1, 1, 1, 0.5)
			end
			if UiTextButton("Quick load", 260, 40) then
				Command("game.quickload")
			end
		UiPop()
		UiTranslate(0, 47)

		if UiTextButton("Restart mission", 260, 40) then
			Restart()
		end
		UiTranslate(0, 47)
		UiTranslate(0, 20)

		if state ~= "win" then
			if UiTextButton("Abort mission", 260, 40) then
				Menu()
			end
			UiTranslate(0, 47)
		end

		if UiTextButton("Main menu", 220, 40) then
			Menu()
		end
		UiTranslate(0, 47)
		UiTranslate(0, 40)

		_, endScreenHeight = UiGetRelativePos()
		UiPop()
		UiPop()
	end
end

function drawTargetDots(count, taken, blinkUntaken)
	UiPush()
	for i = 1, count do
		if taken >= i then
			UiImage("ui/common/score-target-taken.png")
		else
			UiPush()
			if blinkUntaken then
				local a = 0.7 + math.sin(GetTime() * 10) * 0.3
				UiColorFilter(1, 1, 1, a)
			end
			UiImage("ui/common/score-target.png")
			UiPop()
		end
		UiTranslate(-18, 0)
	end
	UiPop()
	return 18 * count
end

function getScoreDetails(score)
	local details = {}
	details.required = mission.required
	details.optional = mission.primary + mission.secondary - mission.required

	if score >= mission.required then
		details.requiredTaken = mission.required
	else
		details.requiredTaken = score
	end

	details.optionalTaken = clamp(score - mission.required, 0, mission.primary + mission.secondary - mission.required)
	return details
end

function drawScore(score, timeLeft, missionTime)
	UiPush()
	UiAlign("left")
	local details
	local eval = false
	if gScoreShown < score + 1 then
		gScoreTimer = gScoreTimer - GetTimeStep()
		if gScoreTimer < 0.0 then
			gScoreShown = gScoreShown + 1
			gScoreTimer = 0.15
			if gScoreShown == score + 1 then
				SetValue("gScoreStatsScale", 1, "linear", 0.5)
				UiSound("ui/common/score-stats-500.ogg")
			else
				eval = true
			end
		end
	end
	if gScoreShown < score then
		score = gScoreShown
	end
	details = getScoreDetails(score)
	if eval then
		if score <= details.required + details.optional then
			UiSound("score.ogg")
		else
			UiSound("ui/common/score-bonus.ogg")
		end

		if score == details.required then
			gScoreTimer = gScoreTimer + 0.2
		end
		if score == details.required + details.optional then
			gScoreTimer = gScoreTimer + 0.4
		end
		if score > details.required + details.optional then
			gScoreTimer = gScoreTimer + 0.2
		end
	end

	UiTranslate(0, 20)
	UiFont("bold.ttf", 24)
	UiColor(0.5, 0.5, 0.5)
	UiTranslate(30, 0)
	UiTranslate(40, 50)

	UiColor(1, 1, 1)
	UiFont("bold.ttf", 22)
	UiText("Targets", true)
	UiTranslate(0, -4)
	UiFont("regular.ttf", 20)

	UiColor(1, 1, 1)
	UiPush()
		UiTranslate(260, -16)
		drawTargetDots(details.required, details.requiredTaken)
	UiPop()
	UiText("Required", true)

	if details.optional > 0 then
		UiPush()
			UiTranslate(260, -16)
			UiColor(0.6, 0.6, 0.6)
			drawTargetDots(details.optional, details.optionalTaken, true)
		UiPop()
		UiText("Optional", true)
	end

	UiTranslate(0, 15)
	UiFont("bold.ttf", 24)
	UiColor(1, 1, 0.7)
	UiText("Stats", true)
	UiTranslate(0, -4)

	UiFont("regular.ttf", 20)
	UiColor(1, 1, 0.7)
	if timeLeft > 0 then
		UiPush()
			if gScoreStatsScale > 0 then
				UiTranslate(280, 0)
				UiAlign("right")
				UiText(math.floor(timeLeft * gScoreStatsScale * 10 + 0.5) / 10 .. "s")
			end
		UiPop()
		UiText("Time left", true)
	end
	if missionTime > 0 then
		UiPush()
			if gScoreStatsScale > 0 then
				UiTranslate(280, 0)
				UiAlign("right")
				local m = math.floor(gScoreStatsScale * missionTime / 60)
				local s = math.floor(gScoreStatsScale * missionTime - m * 60)
				if m > 0 then
					UiText(m .. "m " .. s .. "s")
				else
					UiText(s .. "s")
				end
			end
		UiPop()
		UiText("Mission time", true)
	end
	if timeLeft == 0 and missionTime == 0 then
		UiText("Mission skipped", true)
	end
	_, gScoreCanvasHeight = UiGetRelativePos()
	UiPop()
	return gScoreCanvasHeight
end
