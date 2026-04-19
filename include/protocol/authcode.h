#ifndef __DSPEAK_AUTH_CODE_H
#define __DSPEAK_AUTH_CODE_H

// obsolete operation
/*enum XXOpCode{
	CREATE_SUBCHANNEL_CHK,		// 创建子频道
	DISSMISS_SUBCHANNEL_CHK,	// 解散子频道
	CHANGE_INFO,				// 更改子平道信息
	ADD_MANAGER,				// 添加管理员
	REMOVE_MANAGER,				// 删除管理员
	ADD_MEMBER,					// 添加成员
	REMOVE_MEMBER,				// 删除会员
	JOIN_CHANNEL,				// 加入限制及频道
	JOIN_WITHOUTPASSWD,			// 加入没有密码平道
	KICKOFF,					// 踢人
	VOICE,						// 语音
	CHANGESTYLE,				// 更改模式
	TUOREN,						// 拖人
	SETKEYACTIVE,				// 键盘激活
	DISABLE_VOICE,				// 禁止语音
	ENABLE_VOICE,				// 启用语音
	ENABLE_TEXT,				// 禁止文本
	DISABLE_TEXT,				// 启用文本
	BROADCAST,					// 广播
	DRAGETO_QUEUE,				// 拖进卖序
	JOIN_QUEUE,					// 抢麦	
	LEAVE_QUEUE,				// 退出麦序
	MOVEUP_QUEUE,				// 上移麦序
	MOVEDOWN_QUEUE,				// 下移麦序
	KICK_QUEUE,					// 踢出麦序
	MUTE_QUEUE,					// 禁止抢麦
	UNMUTE_QUEUE,				// 启用抢麦
	DOUBLETIME_QUEUE,			// 双倍麦序时间
	DISABLE_QUEUE,				// 禁用麦序
	ALLOW_QUEUE,				// 启用麦序
	DRAG,						// 拖人
};*/

namespace protocol{
	namespace session{
		struct  session_auth
		{
			enum auth_code{
					none = 0,
					change_room_owner			=3,				//更改频道所有者
					room_broadcast				=4,				//发布所管辖的频道广播
					remove_room					=5,				//删除频道
					edit_room_info				=6,				//修改所管辖的频道信息
					add_remove_child_room		=7,				//在所管辖频道下创建/删除子频道
					add_remove_room_member		=8,				//添加/删除会员(R)
					add_remove_room_admin		=9,				//添加/删除管理员(MAN)
					add_remove_child_room_admin =10,			//添加/删除所管辖的频道的子频道管理员（CA）
					add_remove_room_vip			=11,			//添加/删除贵宾
					add_remove_room_tvip		=12,			//添加/删除临时贵宾
					add_remove_room_ban_list	=13,			//添加/删除黑名单(id/ip)
					kick_user_from_room			=14,			//将用户踢出频道
					kick_all_user_from_room		=15,			//将所有用户踢出频道
					control_user_voice			=16,			//禁止/解除禁止用户语音
					control_user_text			=17,			//禁止/解除禁止用户文字
					control_room_voice			=18,			//禁止/解除禁止所管辖的频道语音
					control_room_text			=19,			//禁止/解除禁止所管辖的频道文字
					exclude_from_member_edit	=20,			//无视添加/删除会员
					exclude_from_admin_edit		=21,			//无视添加/删除频道管理员
					exclude_from_chairman_mode	=22,			//无视主席模式
					exclude_from_vip_edit		=23,			//无视添加/删除贵宾
					exclude_from_tvip_edit		=24,			//无视添加/删除临时贵宾
					exclude_from_mic_order_mode =25,			//无视麦序模式
					exclude_from_limited_room   =26,			//无视限制频道
					exclude_from_voice_ban		=27,			//无视语音禁止4
					exclude_from_text_ban		=28,			//无视文本禁止
					exclude_from_kick_user		=29,			//无视踢人
					exclude_from_passwd_room	=30,			//无视密码
					exclude_from_schedule		=31,			//可选择拒绝调度
					exclude_from_banlist		=32,			//无视黑名单IP/ID
					exclude_from_queue			=33,			//无视排队
					exclude_from_count_limit	=34,			//无视人数限制
					set_others_voice_by_key		=35,			//设置其他人为按键说话
					set_room_voice_by_key		=36,			//设置当前频道为按键发言
					schedule_people				=37,			//调度人员
					change_room_mode			=38,			//更改房间模式
					change_mic_mode				=39,			//禁止/解除禁止抢麦
					change_mic_order			=40,			//上移/下移麦序
					add_remove_from_mic_queue	=41,			//从麦序踢出去
					//mute_mic_queue				=42,			//静音麦序	
					change_mic_time				=43,			//更改麦序时间
					change_mic_owner			=44,			//控麦/解除控麦
					remove_room_from_favor		=45,			//将频道从收藏移除
					withdraw_from_guild			=46,			//退出公会
					room_voice					=47,			//能否语音
					room_text					=48,				//能否输入文本
					schedule_room				=49,			//调度房间
					direct_kick_from_room		=50	,			//直接踢出频道
					exclude_from_room_text_ban = 51,
					change_sess_order		   = 52,				//修改頻道排序
					member_text					=53,                //能否會員聊天
					update_member_car			=54,                //能否修改資料
					max_auth_code				= 55
				};
		};
	}
}


#endif //__DSPEAK_AUTH_CODE_H

