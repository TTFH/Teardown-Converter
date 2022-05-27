import os
import re
import glob

# Parameters with more that 0.05 precision
precision_error = "( (?!(?:rotstrength|steerassist|antiroll|bulge|constant|color))[a-z]+=\"(?:-?\d+\.\d+[ \"])*)(-?\d+\.\d(?:[12346789]|\d{2,}))([ \"])"

default_vals = [
	" acceleration=\"1\.0\"",
	" ambience=\"outdoor/field\.ogg\"",
	" ambient=\"1\.0\"",
	" ambientexponent=\"1\.3\"",
	" angle=\"90\.0\"",
	" antiroll=\"0\.0\"",
	" antispin=\"0\.0\"",
	" autodisable=\"false\"",
	" axis=\"z\"",
	" bloom=\"1\.0\"",
	" brightness=\"1\.0\"",
	" bulge=\"0\.08\"",
	" colorbalance=\"1\.0 1\.0 1\.0\"",
	" constant=\"0\.003 0\.003 0\.003\"",
	" damping=\"1\.0\"",
	" density=\"1\.0\"",
	" depth=\"10\.0\"",
	" difflock=\"0\.0\"",
	" drive=\"0\.0\"",
	" driven=\"false\"",
	" dynamic=\"false\"",
	" emissive=\"1\.0\"",
	" enabled=\"false\"",
	" exposure=\"0\.0 10\.0\"",
	" extrude=\"1\.0\"",
	" foam=\"0\.5\"",
	" fogColor=\"1\.0 1\.0 1\.0\"",
	" fogParams=\"40\.0 100\.0 0\.9 4\.0\"",
	" fogiter=\"1\.0\"",
	" fogscale=\"1\.0\"",
	" friction=\"1\.3\"",
	" fxca=\"0\.0\"",
	" fxglitch=\"0\.0\"",
	" fxnoise=\"0\.0\"",
	" fxraster=\"0\.0\"",
	" gamma=\"1\.0\"",
	" glare=\"0\.0\"",
	#" id_=\"\d+\.0\"",
	" interactive=\"false\"",
	" material=\"none\"",
	" maxstretch=\"0\.0\"",
	" motion=\"0\.5\"",
	" nightlight=\"true\"",
	" offset=\"0\.0 0\.0 0\.0\"",
	#" open_=\"true\"",
	" padbottom=\"5\.0\"",
	" padleft=\"5\.0\"",
	" padright=\"5\.0\"",
	" padtop=\"5\.0\"",
	" pbr=\"0\.0 0\.0 0\.0 0\.0\"",
	" penumbra=\"10\.0\"",
	" pos=\"0\.0 0\.0 0\.0\"",
	" prop=\"false\"",
	" puddleamount=\"0\.0\"",
	" puddlesize=\"0\.5\"",
	" rain=\"0\.0\"",
	" reach=\"0\.0\"",
	" resolution=\"640\.0 480\.0\"",
	" ripple=\"0\.5\"",
	" rot=\"0\.0 0\.0 0\.0\"",
	" rotspring=\"0\.5\"",
	" rotstrength=\"0\.0\"",
	" saturation=\"1\.0\"",
	" scale=\"1\.0\"",
	" shadowVolume=\"100\.0 25\.0 100\.0\"",
	" skybox=\"cloudy\.dds\"",
	" skyboxbrightness=\"1\.0\"",
	" skyboxrot=\"0\.0\"",
	" skyboxtint=\"1\.0 1\.0 1\.0\"",
	" slack=\"0\.0\"",
	" slippery=\"0\.0\"",
	" snowamount=\"0\.0\"",
	" snowdir=\"0\.0 -1\.0 0\.0 0\.2\"",
	" snowonground=\"false\"",
	" sound=\" 1\.0\"", # sound=NULL 1 -> Default
	" soundramp=\"2\.0\"",
	" spring=\"1\.0\"",
	" steer=\"0\.0\"",
	" steerassist=\"0\.0\"",
	" strength=\"1\.0\"",
	" sunBrightness=\"0\.0\"",
	" sunColorTint=\"1\.0 1\.0 1\.0\"",
	" sunDir=\"auto\"",
	" sunFogScale=\"1\.0\"",
	" sunGlare=\"1\.0\"",
	" sunLength=\"32\.0\"",
	" sunSpread=\"0\.0\"",
	" topspeed=\"70\.0\"",
	" travel=\"-0\.1 0\.1\"",
	" unshadowed=\"0\.0\"",
	" waterhurt=\"0\.0\"",
	" wave=\"0\.5\"",
	" wetness=\"0\.0\"",
	" wind=\"0\.0 0\.0 0\.0\"",
	#" name=\"[^\"]*\"",
	"<location[^>]* name=\"between\"[^>]*\/>"
]

def main():
	# For each XML file in current directory
	#for filename in os.listdir("."):
	#	if filename.endswith(".xml"):
	for filename in glob.iglob("**/*.xml", recursive = True):
		file = open(filename, "r+")
		content = file.read()
		new_content = content
		# Convert integers and floats with no integer part to float, convert -0 to 0
		new_content = re.sub("([\" ])(-?\.?\d+)(?=[\" ])", lambda m: m.group(1) + str(float(m.group(2)) + 0), new_content)
		# Remove default values
		for s in default_vals:
			new_content = re.sub(s, "", new_content)
		# Round values to closer 0.05, ex: -.37499999000001 -> -0.35
		#for i in range(4):
		#	new_content = re.sub(precision_error, lambda m: m.group(1) + str(round(float(m.group(2)) * 20) / 20) + m.group(3), new_content)

		# Remove more default values
		new_content = re.sub("(<(?:compound|voxagon|voxscript|vox)[^>]*) collide=\"true\"", "\g<1>", new_content)
		new_content = re.sub("(<joint[^>]*) collide=\"false\"", "\g<1>", new_content)
		new_content = re.sub("(<joint[^>]*) type=\"ball\"", "\g<1>", new_content)
		new_content = re.sub("(<joint[^>]*) size=\"0\.1\"", "\g<1>", new_content)
		new_content = re.sub("(<joint[^>]*) sound=\"false\"", "\g<1>", new_content)
		new_content = re.sub("(<joint[^>]*) limits=\"0\.0 0\.0\"", "\g<1>", new_content)
		new_content = re.sub("(<light[^>]*) type=\"sphere\"", "\g<1>", new_content)
		new_content = re.sub("(<light[^>]*) color=\"1\.0 1\.0 1\.0\"", "\g<1>", new_content)
		new_content = re.sub("(<light[^>]*) size=\"0\.1\"", "\g<1>", new_content)
		new_content = re.sub("(<rope[^>]*) size=\"0\.2\"", "\g<1>", new_content)
		new_content = re.sub("(<rope[^>]*) color=\"0\.0 0\.0 0\.0(?: 1\.0)?\"", "\g<1>", new_content)
		new_content = re.sub("(<screen[^>]*) size=\"0\.9 0\.5\"", "\g<1>", new_content)
		new_content = re.sub("(<trigger[^>]*) type=\"sphere\"", "\g<1>", new_content)
		new_content = re.sub("(<trigger[^>]*) size=\"10\.0\"", "\g<1>", new_content)
		new_content = re.sub("(<vehicle[^>]*) sound=\"medium\"", "\g<1>", new_content)
		new_content = re.sub("(<water[^>]*) type=\"box\"", "\g<1>", new_content)
		new_content = re.sub("(<water[^>]*) size=\"1000\.0 1000\.0\"", "\g<1>", new_content)
		new_content = re.sub("(<water[^>]*) color=\"0\.01 0\.01 0\.01\"", "\g<1>", new_content)
		new_content = re.sub("(<voxbox[^>]*) size=\"50\.0 30\.0 20\.0\"", "\g<1>", new_content)
		new_content = re.sub("(<voxbox[^>]*) color=\"1\.0 1\.0 1\.0\"", "\g<1>", new_content)

		new_content = re.sub("version=\"\d\.\d\.\d\"", "version=\"1.0.0\"", new_content)
		new_content = re.sub("( texture=\"\d+\.0) 1\.0(\")", "\g<1>\g<2>", new_content)
		new_content = re.sub("( blendtexture=\"\d+\.0) 1\.0(\")", "\g<1>\g<2>", new_content)
		new_content = re.sub("([\" ])(-?\d+)\.0(?=[\" ])", "\g<1>\g<2>", new_content)

		if content != new_content:
			print(f"Updating file {filename}...")
			file.seek(0)
			file.write(new_content)
			file.truncate()
		file.close()

main()
