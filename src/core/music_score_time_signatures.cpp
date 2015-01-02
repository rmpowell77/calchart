#include "music_score_time_signatures.h"
#include "music_score_moment_browser.h"
#include "music_score_jumps.h"

TimeSignaturesCollection::TimeSignaturesCollection(TimeSignature defaultEvent)
: super(defaultEvent)
{}

MusicScoreMomentBrowser* TimeSignaturesCollection::makeMusicScoreMomentBrowser(const MusicScoreJumpsCollection* jumps, MusicScoreMoment startTime) const {
	if (jumps == nullptr) {
		return nullptr;
	}
	return new MusicScoreMomentBrowser(this, jumps, startTime);
}