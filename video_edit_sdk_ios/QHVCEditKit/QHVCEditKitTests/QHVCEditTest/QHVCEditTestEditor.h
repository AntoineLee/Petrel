//
//  QHVCEditTestEditor.h
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/5.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QHVCEditKit/QHVCEditKit.h>

int QHVCEditTestEditorAll(void);

int QHVCEditTestEditorTimelineAll(void);
QHVCEditTimeline* QHVCEditTestEditorCreateTimeline(void);
int QHVCEditTestEditorFreeTimeline(QHVCEditTimeline* timeline);

int QHVCEditTestEditorTrackAll(void);
QHVCEditSequenceTrack* QHVCEditTestEditorCreateSequenceTrack(QHVCEditTimeline* timeline);
QHVCEditSequenceTrack* QHVCEditTestEditorCreateAudioTrack(QHVCEditTimeline* timeline);
QHVCEditOverlayTrack* QHVCEditTestEditorCreateOverlayTrack(QHVCEditTimeline* timeline);

int QHVCEditTestEditorEffectAll(void);
QHVCEditEffect* QHVCEditTestEditorCreateEffect(QHVCEditTimeline* timeline);

int QHVCEditTestEditorClipAll(void);
QHVCEditTrackClip* QHVCEditTestEditorCreateVideoClip(QHVCEditTimeline* timeline);
QHVCEditTrackClip* QHVCEditTestEditorCreateImageClip(QHVCEditTimeline* timeline);
QHVCEditTrackClip* QHVCEditTestEditorCreateAudioClip(QHVCEditTimeline* timeline);
