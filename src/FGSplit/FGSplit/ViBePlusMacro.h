#ifndef VIBEPLUSMACRO_H
#define VIBEPLUSMACRO_H

/*		默认参数		*/
#define DEFAULT_NUM_SAMPLES		20		// 默认样本数

#define DEFAULT_MIN_MATCH		2		// #min指数

#define DEFAULT_RADIUS			5		// Sqthere匹配半径

#define DEFAULT_THRESHOLD_MIN	20		// 分类阈值下限

#define DEFAULT_THRESHOLD_MAX	40		// 分类阈值上限

#define DEFAULT_COLOR_THRESHOLD	20		// 颜色畸变阈值

#define FILL_UP_AREA_SIZE		50		// 更新蒙版中前景孔洞填充大小

#define FILL_SEG_AREA_SIZE		20		// 分割蒙版中前景孔洞填充大小

#define DEL_SEG_AREA_SIZE		10		// 分割蒙版中前景斑点抹除大小

#define DEFAULT_RAND_SAM		20		// 子样采集

#define DEFAULT_FG_COUNT		50		// 连续前景转背景上限值

#define AMP_MULTIFACTOR			0.5		// 振幅乘数因子

#define ID_BG_INNER				21		// 背景内边缘

#define ID_INNER_STATE			22		// 在背景内边缘状况下，8邻域状态位

#define ID_BLINK_LEVEL			23		// 闪烁等级

#define ID_MAX_INNER_GRAD		24		// 邻域梯度最大值

#define MAX_INNER_GRAD			50		// 邻居灰度最大梯度

#define MINUS_BLINK_LEVEL		1		// 闪烁等级减少量

#define ADD_BLINK_LEVEL			15		// 闪烁等级增加量

#define TC_BLINK_LEVEL			30		// 闪烁等级阈值

#define MAX_BLINK_LEVEL			150		// 最大闪烁等级

#endif // VIBEPLUSMACRO_H
