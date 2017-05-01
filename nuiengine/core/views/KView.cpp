﻿//  **************************************
//  File:        KView.cpp
//  Copyright:   Copyright(C) 2013-2017 Wuhan KOTEI Informatics Co., Ltd. All rights reserved.
//  Website:     http://www.nuiengine.com
//  Description: This code is part of NUI Engine (NUI Graphics Lib)
//  Comments:
//  Rev:         2
//  Created:     2017/4/11
//  Last edit:   2017/4/28
//  Author:      Chen Zhi
//  E-mail:      cz_666@qq.com
//  License: APACHE V2.0 (see license file) 
//  ***************************************
#include "KView.h"
#include "DataSync.h"
#include "KScreen.h"
#include "KTextView.h"
#include "KViewGroup.h"
#include "boost/lexical_cast.hpp"
#include "KFontManager.h"
using namespace std;
using namespace boost;
using namespace boost::gregorian;

int g_view_count = 0;
KView::KView()
{
	g_view_count++;
//	m_ref = 0;
	m_e_viewtype = KVIEW_BASE;

	m_b_focus = false;
	m_b_active = false;
//	m_p_parent = NULL;
	m_p_screen = NULL;
	m_b_do_msg = TRUE;
	m_shield_msg = TRUE;
	//m_sign_move.connect(this, &KView::OnMove);
	//m_sign_down.connect(this, &KView::OnDown);
	//m_sign_up.connect(this, &KView::OnUp);
	//m_sign_dclick.connect(this, &KView::OnDClick);
	//m_sign_rdown.connect(this, &KView::OnRDown);
	//m_sign_rup.connect(this, &KView::OnRUp);
	//m_sign_rdclick.connect(this, &KView::OnRDClick);
	//m_sign_wheel_down.connect(this, &KView::OnWheel);


	//m_sign_touchmove.connect(this, &KView::OnTouchMove);
	//m_sign_touchdown.connect(this, &KView::OnTouchDown);
	//m_sign_touchup.connect(this, &KView::OnTouchUp);

	//m_sign_gesture.connect(this, &KView::OnGesture);
	m_p_gesture_detector = new KGestureDetector;

	m_b_enable_gesture = false;

	m_pSurface = NULL;
	m_b_cache = FALSE;
	m_b_update_cache = TRUE;

	m_b_mouse_picked = false;
	m_b_rmouse_picked= false;
	m_dw_touctpoint_count = 0;

	m_b_mouse_in_view = false; 
	m_i_modal_result = 0;
	m_rotate_angle = 0;
	m_rotate_point.set(0,0);

	m_rect.setXYWH(0,0,100,50);

	m_b_modal_exit = false;
	m_i_modal_result = 0;

	m_transform_flag = 0;

	m_scale_size.fX = 1;
	m_scale_size.fY = 1;

	m_LRMethod_left = KVIEW_LRMethod_Parent_LRNone;
	m_LRMethod_right = KVIEW_LRMethod_Parent_LRNone;
	m_BTMethod_top = KVIEW_BTMethod_Parent_BTNone;
	m_BTMethod_bottom = KVIEW_BTMethod_Parent_BTNone;

	m_b_show_tip = FALSE;
	m_tip_id = NO_TIMER;

	m_name = lexical_cast<kn_string>(g_view_count);
	m_id = g_view_count;

	m_tip_delay_time = NO_TIMER;
	m_tip_hide_time = NO_TIMER; 
}
 
KView::~KView()
{
	g_view_count--;
	Release();
}

void KView::shared_ptr_inited()
{
	m_sign_move.connect(this, &KView::OnMove);
	m_sign_down.connect(this, &KView::OnDown);
	m_sign_up.connect(this, &KView::OnUp);
	m_sign_dclick.connect(this, &KView::OnDClick);
	m_sign_rdown.connect(this, &KView::OnRDown);
	m_sign_rup.connect(this, &KView::OnRUp);
	m_sign_rdclick.connect(this, &KView::OnRDClick);
	m_sign_wheel_down.connect(this, &KView::OnWheel);


	m_sign_touchmove.connect(this, &KView::OnTouchMove);
	m_sign_touchdown.connect(this, &KView::OnTouchDown);
	m_sign_touchup.connect(this, &KView::OnTouchUp);

	m_sign_gesture.connect(this, &KView::OnGesture);
}
 
void KView::setName(const kn_string& name)
{
	m_name = name;
}

kn_string& KView::getName()
{
	return m_name;
}

void KView::setID(kn_int id)
{
	m_id = id;
}

kn_int KView::getID()
{
	return m_id;
}

KVIEWTYPE KView::getType()
{
	return m_e_viewtype;
}

void KView::Release()
{
	
	KScreen* p =  GetScreen(); 
	KView_PTR v_null;
	if (!p)
	{
		p=p;
	}
	//if (m_b_active)
	//{
	//	if (p) 
	//	{
	//			p->setViewActive(v_null, FALSE);
	//	}
	//}
	if (m_b_focus)
	{
		if (p)
		{
			p->setFocusView(v_null, FALSE);
		}
	}


	writeLock lock(m_lst_drawable_mutex);
	m_lst_drawable.clear();	
	m_sign_move.disconnect_all();
	m_sign_down.disconnect_all();
	m_sign_up.disconnect_all();
	m_sign_dclick.disconnect_all();
	m_sign_rdown.disconnect_all();
	m_sign_rup.disconnect_all();
	m_sign_rdclick.disconnect_all();
	m_sign_wheel_down.disconnect_all();


	m_sign_touchmove.disconnect_all();
	m_sign_touchdown.disconnect_all();
	m_sign_touchup.disconnect_all();

	m_sign_gesture.disconnect_all();


	SAFE_DELETE(m_p_gesture_detector);

//	m_p_parent = NULL;
	m_p_screen = NULL;
	SAFE_DELETE(m_pSurface);


}

bool KView::Create(const RERect& rect)
{
	return Create(rect.left(), rect.top(), rect.width(), rect.height());

}

kn_bool KView::Create(kn_int iX, kn_int iY, kn_int iWidth, kn_int iHeight)
{
	m_rect.setXYWH(iX, iY, iWidth, iHeight);
	m_transform_flag = 0;
	RERect rect; 
	rect.set(0,0, iWidth,iHeight);
	InvalidateRect(rect);
	//SaveOriginalViewRect(m_rect);
	return true;
}

kn_uint  KView::addDrawable(KDrawable_PTR  p )
{
	writeLock lock(m_lst_drawable_mutex);
	m_lst_drawable.push_back(p);
	return m_lst_drawable.size()-1;
}

void KView::Draw(IRESurface*  pDstSurface, kn_int x, kn_int y)
{
	//RERect  rect = GetBoundRect();
	//rect.offset(x,y);

	RefreshSurface(pDstSurface, x, y);//rect.left(), rect.top());

#ifndef _WIN32_WCE 
	clearInvalidRect();
#endif
}


void KView::enableCache(kn_bool b)
{
	if (m_b_cache == b)
	{
		return;
	}
	if (b)
	{//打开cache 创建缓冲
		if (m_pSurface == NULL)
		{
			RERect rct = GetBoundRect();
			m_pSurface = RESurfaceFactory::CreateRESurface(rct.width(), rct.height(), REBitmap::kARGB_8888_Config);
		}

	}
	else
	{
		SAFE_DELETE(m_pSurface);
	}
	m_b_cache = b;
}

// 局部刷新
void KView::Draw(IRESurface* pDstSurface, LSTRECT& lst_rect, kn_int x, kn_int y)
{
	if (m_b_cache)
	{//有缓冲
		if (m_b_update_cache)
		{
			RefreshSurface(m_pSurface, 0, 0);
			m_b_update_cache = FALSE;
		}
		if (m_transform_flag == 0)
		{
			pDstSurface->DrawBitmap(m_pSurface, x, y);
		}
		else
		{
			REMatrix m ;
			getMatrix( x, y, m);
			pDstSurface->DrawBitmapMatrix(m_pSurface, m);
		}
	}
	else
	{
		RefreshSurface(pDstSurface, x, y);
	}

	clearInvalidRect();
}


void  KView::showMenu(KMenuBase_PTR p_menu, int ix, int iy)
{
	int x,y, mx, my;
	GetScreenXY(x,y);
	mx = x + ix;
	my = y + iy;
	//保证menu在界面内
	if (mx + p_menu->GetRect().width() > GetScreen()->GetWidth() )
	{
		mx -= p_menu->GetRect().width();
	}
	if (my + p_menu->GetRect().height() > GetScreen()->GetHeight() )
	{
		my -= p_menu->GetRect().height();
	}

	p_menu->SetPosition(mx, my);
	GetScreen()->showMenu(p_menu);
	
}

void KView::RefreshSurface(IRESurface* pDstSurface, kn_int x, kn_int y)
{
	// 屏幕坐标转groupview的坐标
	int screenx, screeny;
	GetScreenXY(screenx, screeny);
	KScreen* pScreen = GetScreen(); 
	if (!pScreen)
	{
		return;
	}
	RERect  rectScreen = RERect::MakeXYWH(-screenx, -screeny, pScreen->GetWidth(), pScreen->GetHeight());


	//加写锁
	readLock lock(m_lst_drawable_mutex);

	if(m_b_clip)
	{
		pDstSurface->Save();
		pDstSurface->ClipRect(GetScreenRect(m_rect_clip) );
	}

	REMatrix matrix;
	getMatrix(x, y, matrix);

	VEC_DRAWABLE::iterator itEnd = m_lst_drawable.end();
	for(VEC_DRAWABLE::iterator ite = m_lst_drawable.begin();ite != itEnd; ++ite)
	{
		KDrawable_PTR p = (*ite);

		if( !p->isShow() || !RERect::Intersects(rectScreen, p->GetRect()) )
		{
			// 不绘制屏幕以外的drawable
			continue;
		}

		int i_ds_alpha = p->GetPaint()->getAlpha();
		int i_alpha =  i_ds_alpha * m_i_opacity *(p->getOpacity()) / 65025; //(255*255)
		p->GetPaint()->setAlpha(i_alpha);
		if (p->getType() == KDrawable::KDRAWABLE_COLOR)
		{
			((KColorDrawable*)p.get() )->setColorAlpha( i_alpha );
		}
 
		if (m_transform_flag == 0)
		{
			p->Draw(pDstSurface, NULL, m_rect.left()+ x, m_rect.top() + y);
		}
		else
		{
			p->Draw(pDstSurface, &matrix, m_rect.left()+ x, m_rect.top() + y);
		}


		// 还原
		p->GetPaint()->setAlpha(i_ds_alpha);
	}

	if(m_b_clip)
	{
		pDstSurface->Restore();
	}
}

void KView::RefreshCache()
{
	m_b_update_cache = TRUE;
}


kn_bool KView::IsMousePicked()
{
	return m_dw_touctpoint_count > 0;
}

kn_bool KView::DoFocusMessage(KMessage* pMsg)
{//焦点控件消息处理,多一个坐标转换过程,因为焦点控件直接在screen层响应
	int x = 0;
	int y = 0;

	if ( !m_p_parent.expired())
	{
		m_p_parent.lock()->GetScreenXY(x, y);
	}

	if (pMsg->HasPos())
	{
		((KMessagePos*)pMsg)->m_pos_x -=x;
		((KMessagePos*)pMsg)->m_pos_y -= y;
	}
	kn_bool ret = DoMessage(pMsg);

	if (pMsg->HasPos())
	{	
		((KMessagePos*)pMsg)->m_pos_x += x;
		((KMessagePos*)pMsg)->m_pos_y += y;
	}

	return ret;
}

kn_bool KView::isPointInView(int x, int y)
{// 检查点是否在view内部，除了view范围，还有裁剪区处理，应统一使用这个函数
	if(GetBoundRect().contains( x, y) )
	{
		if( m_b_clip )
		{
			if (m_rect_clip.contains( x,y))
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}

	return false;
}
kn_bool KView::DoMessage(KMessage* pMsg)
{
	if (pMsg->m_msg_type == KMSG_3DX) {
		int i = 0;
	}
	if (!m_b_do_msg || !m_b_show)
	{
		return false;
	}

	if(pMsg->HasPos() && !(m_b_mouse_picked || m_b_mouse_in_view ) )
	{//带位置的消息，在view之外原则上不响应，如果之前是 m_b_mouse_picked 或m_b_mouse_in_view 状态，则不管在不在view上都处理
		if( !isPointInView( ((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y))
		{
			return false;
		}
		else
		{

		}
	}

	//设置当前处理消息的view
	pMsg->m_p_view = shared_from_this();

	switch(pMsg->m_msg_type)
	{//这里的处理是防止部分位置消息被吃掉
	case KMSG_LBBUTTONDOWN:
	case KMSG_TOUCH_DOWN:
	case KMSG_LBUTTONDBLCLK:
	case KMSG_RBUTTONDBLCLK:
	case KMSG_RBBUTTONDOWN:
		if ( !isPointInView(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y) )
		{
			return false;
		}

	}

// 	if((pMsg->m_msg_class_type == KMSG_TYPE_TOUCH ) && ((KMessageTouch*)pMsg)->m_touch_id != m_dw_touch_id)
// 	{
// 		return false;
// 	}



	kn_bool ret = FALSE;

	// touch消息会伴随着LBUTTON，有需要时可以屏蔽鼠标消息
	switch(pMsg->m_msg_type)
	{
	case KMSG_MOUSEMOVE:
 
		handleMouseMove(pMsg);
		ret = FALSE;
		break;
	case KMSG_LBBUTTONDOWN:
		handleMouseDown(pMsg);
		ret = m_shield_msg;
		break;
	case KMSG_LBUTTONDBLCLK:
 
		m_sign_dclick.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);
		ret = m_shield_msg;
		break;
	case KMSG_LBBUTTONUP:
		//MyTraceA("KMoveableView::DoMessage KMSG_LBBUTTONUP ID = %d", m_dw_touch_id) ;
		handleMouseUp(pMsg);
		 
		break;
	case KMSG_RBBUTTONDOWN:
		m_b_rmouse_picked = TRUE;
		m_sign_rdown.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y,(KMessageMouse*) pMsg);
		ret = m_shield_msg;
		break;
	case KMSG_RBUTTONDBLCLK:
		m_sign_rdclick.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);
		ret = m_shield_msg;
		break;
	case KMSG_RBBUTTONUP:
		if (m_b_rmouse_picked)
		{
			
			m_sign_rup.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);
			m_b_rmouse_picked = FALSE;
		}
		break;
	case KMSG_MOUSEWHEEL:
		onWheelDirect((KMessageMouseWheel*)pMsg);
		m_sign_wheel_down.emit( (KMessageMouseWheel*)pMsg);
		break;
	case KMSG_KEYDOWN:
		m_sign_key_down.emit((KMessageKey*)pMsg);
		break;
	case KMSG_TYPE_INPUT_ING:
	case KMSG_TYPE_INPUT_END:
	case KMSG_TYPE_CHAR:
	case KMSG_TYPE_IME_UPDATE:
		m_sign_keyboard_input.emit((KMessageInput*)pMsg);
		break;
	case KMSG_KEYUP:
		m_sign_key_up.emit((KMessageKey*)pMsg);
		break;
	case KMSG_3DX:
		m_sign_3dx.emit((KMessage3Dx*)pMsg);
		break;
	case KMSG_TOUCH_DOWN:
		{
			ret = m_shield_msg;
			KMessageTouch* pTouchMsg = (KMessageTouch*)pMsg;
			if(pTouchMsg->m_iPointCount == 1)
			{
				m_dw_touctpoint_count = 1;
				
				// 按 KMSG_LBBUTTONDOWN 处理
				handleMouseDown(pMsg);
			}
			else if(pTouchMsg->m_iPointCount == 2 && m_dw_touctpoint_count == 1)
			{
				// 1 TOUCH_DOWM 1point
				// 2. TOUCH_DOWM 2point   进入多点时放弃原有单点的操作
 
				handleMouseUp(pMsg);
				ret = FALSE;
			}

			if(m_dw_touctpoint_count < pTouchMsg->m_iPointCount)
			{
				m_dw_touctpoint_count = pTouchMsg->m_iPointCount;
			}
			
			
			//ret = TRUE;
			onTouchDownDirect(pTouchMsg->m_pos_x, pTouchMsg->m_pos_y, pTouchMsg); // 指针强制转换
			m_sign_touchdown.emit(pTouchMsg->m_pos_x, pTouchMsg->m_pos_y, pTouchMsg);
			
		}
		
		break;
	case KMSG_TOUCH_MOVE:
		{

			KMessageTouch* pTouchMsg = (KMessageTouch*)pMsg;
			
			if(pTouchMsg->m_iPointCount == 1 && m_dw_touctpoint_count == 1)
			{
				handleMouseMove(pMsg);				 
			}
			else if(pTouchMsg->m_iPointCount == 2 && m_dw_touctpoint_count == 1)
			{
				// 1 TOUCH_DOWM 1point
				// 2. No TOUCH_DOWM 2point [没有经过TOUCH_DOWM 2point 的情况] 
				// 3 TOUCH_MOVE 2points,   进入多点时放弃原有单点的操作
				handleMouseUp(pMsg);	
				 
			}

			if(m_dw_touctpoint_count < pTouchMsg->m_iPointCount)
			{
				m_dw_touctpoint_count = pTouchMsg->m_iPointCount;
			}
	 
			onTouchMoveDirect(pTouchMsg->m_pos_x, pTouchMsg->m_pos_y, pTouchMsg);
			m_sign_touchmove.emit(pTouchMsg->m_pos_x, pTouchMsg->m_pos_y, pTouchMsg);
			ret = FALSE;
		}
	
		break;
	case KMSG_TOUCH_UP:
		{
			KMessageTouch* pTouchMsg = (KMessageTouch*)pMsg;
			if(pTouchMsg->m_iPointCount == 1 && m_dw_touctpoint_count == 1)	//	确保一直只有1个手指
			{
				// 按 KMSG_LBBUTTONUP 处理			 
				handleMouseUp(pMsg);
				 
			}
	 
			onTouchUpDirect(pTouchMsg->m_pos_x, pTouchMsg->m_pos_y, pTouchMsg);
			m_sign_touchup.emit(pTouchMsg->m_pos_x, pTouchMsg->m_pos_y, pTouchMsg);
			//ret = m_shield_msg;
			m_dw_touctpoint_count = 0;
		}
		
		 
		break;
	case KMSG_DRAG:
	case KMSG_DRAG_UP:
		 onDragDirect((KMessageDrag*)pMsg);
		 break;
	default:
		ret = false;
		if (pMsg->m_msg_type > KNUIMSG_USER)
		{//应用层自定义消息
			ret = OnUserMsg(pMsg);
		}
		break;
	}
	return ret;
}


void KView::handleMouseDown(KMessage* pMsg)
{
	m_b_mouse_picked = true;
	m_dw_touctpoint_count = 1;
	//	g_b_has_picked = true;
	onDownDirect(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y,(KMessageMouse*)pMsg);
	m_sign_down.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y,(KMessageMouse*)pMsg);
}

void KView::handleMouseMove(KMessage* pMsg)
{
	onMoveDirect(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);
	m_sign_move.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);
}

void KView::handleMouseUp(KMessage* pMsg)
{
	if (m_b_mouse_picked)
	{
		onUpDirect(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y,(KMessageMouse*)pMsg);
		m_sign_up.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);

		kn_bool b_in = isPointInView( ((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y ) ;
		if (b_in)
		{
			m_clicked_pos_signal.emit(((KMessagePos*)pMsg)->m_pos_x, ((KMessagePos*)pMsg)->m_pos_y, (KMessageMouse*)pMsg);
			m_clicked_signal.emit(shared_from_this());
		}
		m_b_mouse_picked = false;
		m_dw_touctpoint_count = 0; 
	}
}


void KView::onDragDirect(KMessageDrag* mag)
{

}

kn_bool KView::OnUserMsg(KMessage* pMsg)
{
	return FALSE;
}

void KView::setViewFocus()
{
	KScreen* p= GetScreen();
	if (p)
	{
		p->setFocusView(shared_from_this());
		m_b_focus = TRUE;
	}

	m_sign_focus.emit();
}

void KView::unSetViewFocus()
{
	KScreen* p= GetScreen();
	if (p && p->getFocusView() == shared_from_this())
	{
		KView_PTR v_null;
		p->setFocusView(v_null, FALSE);
	}

	m_b_focus = FALSE;
	m_sign_unfocus.emit();
}

kn_bool KView::bViewFocus()
{
	return m_b_focus;
}

void KView::setViewActive()
{
	//KScreen* p= GetScreen();
	//if (p)
	//{
	//	p->setViewActive(shared_from_this());
	//	m_b_active = TRUE;
	//}
	m_sign_active.emit();
}

void KView::unSetViewActive(kn_bool unsetScreen)
{
	//if (unsetScreen)
	//{
	//	KScreen* p= GetScreen();
	//	if (p)
	//	{
	//		KView_PTR v_null;
	//		p->setViewActi��q6ރvXw����Z��L�5�[����kB��뎆�j�Z� >�C ����X9Z�S��B��d&��6�@"��A�����e�E����p�Z������1�[ivϲǚ�!y�]�}���9��β�1�Ys��.���A����9���l��ℾ�yj��)�&v�*�b S����f�p���淉��4�'⓶���-��^1��}H��tkV���!����WB���5YΟ=����eQ�>�W��ia'wz��p�،EA�I`�Q����
�/a�o@�b��u�UJ7q���0����\_�\��`	����rv2Z���Cyw`[�>}���Җ$@+��B�D���s��f	;��F)M���
y �x7W�E�����.8ԭh�R��h=����<�5�
Z�
�T%]�F�:$���6�\Ĳ34u���h�1>ǂ.z������;��c�1����5d�4�p�����NUz����8 �-	�-/+���e2��C�-}�+�.;QP)Xf`'�)�jN�d���(��j#D���=ETh��j!n�O�>�@~hC�$	t[�L ��p}�o��8!%��̘\*� _�� ����S�+��X�)y�qn,��ʬ��w�H��3�O�wS�6Y�(��l��������&�B^oY�����h��}��mp
c J  M9��*PQ�֏�ɬ�"�����7饙�]9eҽ��>ؔ�F��#�$�y�IV~P��^#[��.o��q^�'�I��݃��o
��]@�7���o.����b@���j&w���U��[͙ֹ����c�eX�ct����U�����܌�{�F8����u�B1f��fAw[���:p^�5�+�!�"c Q�N#��#o6����g���3Hi�CZ�������r�'
��T�L��(h��F��f�0o������r
Z#Fa�n��й�m�j����93�3�
��G\6(���6����!.+�'����vAZ/u`����FUSm�>y3`(�%S^��z�����{@T<
u��z�e�i����a����@e���RزH�#
�r��Sd2Ċj�\)(
���\C���P=@@�iL�$�����]�iB�ӵ�*51Q��������`Ԩ���w2E-
���borC���s�x�(X������Jo��i�@E�-�:�^h2�Ϳ�*�ؑqS��B�g��5��]&P0�6�">����Ԏt���n��@3���:��A� ���]���͚��da&�X]y4�'�:����:_��M�Ż�]��Lr��خlR�3ChyQr�k��Ը��/I�G�D�^/�)��=u9��AT�4��l�	���%�'���g�-aWS�N��G;�=���{!�5"�į$G�l�QLW��DXNF�O#-���"�9�4�Sp�+����b�:ԦYKT{a�(��c8S���[��o>�2E��	��w��1��HP��1(�ڗ����B5 Psf��s�jg|	�a����Q�.Լ��}� ����V��3�,_�1�gq�g��p ��*W��<��@��a����*L-��̬�%T��V��Z�����A�7�cm�kJ�/�OYU��!��b���?Yzϭ,�v!���Py�6[����`:祠34S�3�����a7v"�dz�]��m�qk���gl:۹c�E4�M�yP�����]A3�>a�<��?W:�魫�:�ߎh"N���xd%5��$v��uR�W����
�N�wC�~Α����	�z���5�hކ��s5�,�[�~�S��5�.�8���8KLK�^���C�@Q�����aҳ����U޴�)���J�b1���6`_���u�������h[Op�9:�� �;v@#����cfͼZ�59氭\=u�;;�|���u��Qx`�x$���nl�M��ȱ��+�̬��؛��Y�"���ƆLq�FC7�l% ���M.�B4�'�h/re7���hߍ9fo�\� &�5P�~�
n�K��&�s呰o���6�i�����Γ܊6-���	�O�����~0��d fqe�G;�E�~��f������К��?�P�(��b+�h�Z�B�E=r��Y�/�M�i���ߘ�g�Slr� g�@������W����quJX�ܳF��X5ꄵqP"��$F_�>�Q�κ1��S�T�U��6϶��i�w ����t��M�)�[pEм�S�������k�I�^�-��H�������q��1�{��}��P�
�<��ҡ�ܒ7y�1�h���!u�dOJ�W��[�널��!s���H�vO1�=F�����Y6�yqʨ�@T2�D�nIb�
�l�j`��v|
����?��O%��Gl����Gu'1FA��@\Y2�VS
�$OT�U�8��^��H[0P[s�\�;>8�E�c�+�hlt���[B�f�#�P����E�$Vx؊�6NV�]
��y��o��䴝��c���c��a5�D���m�~I�ɕ1�X��*h�~�S׹'P�(�������ź��	�SuF��3?�i~��:�,c*|��&UY��b-��ky�+db���9o���F��:#�Q����P��o����ޜ� �:)�}���&���az��@V+�(2
����a�RPu:���4*F�u��ʙ�`���S�W�!o�\D�\�τ����B=���r�Q�_	��+����x��ݻ�m�<_σI �����(T�T������g��A8Nu>��}7兩��+�.���ʶ��@2���@���B�\��=zH)�%@�,n$�W!�}��*��ʌ����-�8������O�7&7߅i���ϬP1Y-��}�������ٮ�z`�Kt����-õ/9(��?ܝEaS�����z{c�C�b��J�ɮh	�PU�����R�d?TNP��a�p��r	^�������\UO��@�]X�o�2՛I���B-�'V�����=��� wl�����ҳN��o1�I�]κ%V��Ft�ad��/�����b8>:��݂+�������?�����Ԣ�N>���u��z�N�7t-�W���gx:u+W^U[�@S/�X>�a
��;���bG��ˍ�G
������j��r�/V�EC�n��Ѡ:�S�(�㪜a�>2�c���������9�?Ý������s&���/-�Z�|�K����@�i�@�P-�}�m6�s�W��]�B\�������8�(�3���L�r.c����6��~�p�dZ쾚�(���2��I�2��Åז��߂C�����#�*9���K2n��mG��Q����9>���+`��o�)#��uW�&�.��1ǤaB��0x���%)?��l}jO�hnr�.Z�E�=� a�BB?\_}�7�~����[!�d&wP��_딑.�L&�F����1Hm���@2ץd�@-���շE3��ܔHv��s�It��τL3�X���I��6X�6�طy���H`9�N�ޯ�|L��b�M
 Qc��y���EZ=�*�6��*?3׍���6竺A59	)3z���8x`�g
���I�=�s�����Pܛ�֤@Q��{&:��b\HtM�v��ϝ����ӬHʂv�}�!.Ѵ���B�\T����Etz����g���_$����/���ѠR^9����XD�~���bg`q�%�̬c�����C�zх�{'�����S
#�)�4����,�t�VOm�9�čc�E�˭�'�z�u7��-����z�BE��~bM �Bʒ�s�W�u7ԡs`��}����ͽ
�ʉ�o�!Y����|Z�в�`�#��-Mz�2٩���~���Q�|�]$����:�T^�U18��� ��J�0>WqZ �
�}�Q_�@�U���I.j�ZH�;�ұq@9u���uy�m��'�H����w�AA��!Z��&�;ƛ�d�O�7KǭAȄ��G�:�y��U]e<;7��O����c���w��
8Is�o��L��7���R�^�JQ�9P�"������m5T���5����'��B#e�!	��Z� TӱW
���T�3W�
�E�eL��ɴ����9��)�i��
�6a0�ml�����K}���!{H1�(�7�4���|�S�}�BV�YrI��L��x۩��a��tZ�����H]��p7ԡ���Cp�ꄽ^��7ҳ�D��猩�U�z8{7 ha�_\�v�l�C��^��h����_���E�O�D��F]�M��N�����s��l� �8�T/6��x��ꥼ2l��b����+B$Q�tX�A� iX*P��9J�>"��4�����Q�ߜ���T�TÈ��c�lS���@�kON$�vo�wa�L�w&���e��R#TD�c�Y9����N�TuâU\�ew[+E�Kv\@2!��}~Eÿ�rw�ǄT�x�zT���q����鼁>C����m��h���[#j.���Ө0��ˌ���e�:<\}�3N�r�c"�Z{��Kh���ޜ7��f%F�雥?���5T�<�cp�aP�t�#�+O�/��(�9�'U����	z4���
u���B�\ڱ��ފo#�9|��z���֡��R�uE��CUɴv,���qu�{�ee�Nf�i, �~�H�H-o��4	Qp�]u��h�Y�2fgk�˺v y�>��J���d��W�?��J�$㕅у�y�ui*~�0΃�{������(��ϺR\����?�t�o�@Mdo�b��7��[�8Z�1��PB\"��Fg�d��*��O�	��B���'~g����x/���%��]��A���nq�25L@��d^g��H��|	g���j�Z>b񺂨�uA�`�u��hw����SԏL����l��g�)��"��Bp����N�Q����lW��K3��~5�po���1�ؽ;\�8��,�*�Ma���������}�im�A���~i�}�
Sp4@���7�\g޵
��Myv،-`�#���"�s]s�K=���0�q�Dg����V��nE�:]w�	ZT�v+�xJA�l?At�*�ԄR�i���Ą�Ku���3�r�pe��x�q�h�P�	���~ۢn>�
�S��}�"d��6\\6͹�o���W��Ga·e�y_�	���x)��:s����5�>A;�@ȅ��jeFx=�i~}o	�\B�U!����q,�֚t����b��o�1�C��ȷ:U�g�tv�J� W��P�'�+�*T~c���x݅�5���(���WE����� x��b���[T�Ke$�ؿ� h٫	�Y�-Uz��W�P�|X��E����P�i��\y������=P�&��fI
�:lq(��y5���,e�L��wD��}"�@��l����#�,{�G����N7Md
�a��,����~t`�)�J�JW02�M�˄f���x|4\2,�rESD6�ț�<S}���'���
ӷ^�m��)&��Hd��b���-���j�N"����Y8`��ްI}�S�?�<Mj�6��ZЕ�����Xxu�[t�.ǵ�TpJ��;. ���Udrm����Ҽ�H�h�\�@��J�4��u���_
����6���ne)�;�
}	���I�Q=_pBM�K���[p���Q��-���R)u�9����=�*CHB��ƴTg�^wx�Fz]^'%��Mq�f��uq��=щ�T��2�{��Y�[5��AgUBּ�J.��^>`��M�kNH�l�l2��k2���g;v1ON��L�|ǁ_8-�vN���h"��S#���D-��E�s(��`o�vk(�)�t*`1d�2{$"p��.uƲh��F@�t��9xS*~�I0-'�7o�L��m�?�V~&�^�3^`d����S�_�jj�/�$�U<y3�]`�q�/z��Q�|�y3Y�;��ͻ��̰��CR��p��`e���=J�I�~��b��(�⬌��@Ϧ����ab7}��|mP��^�	��|5,-���S��H�\i�ēL��,�&~A>���,�C�I i��
�ݧ��v���f����.�;�{�4�D0,�]�@�:�x��;&����"g�
���n�R����h!�
��d���F[�,s�C~��U��Ұ���	D�ӗu��n�Z���G[]}�U>nIȚ'����:�a��"�
�;�P΃�C�dhJ��X?mԃ����n�GIN��X�h������(��
B����i�Ӭ@W�c�]=X�;��ۋ�#M\|�L;c~ni%�
!L�\vVO7՚��(TZ�ǽX����Q �S���MJ���t�y<�����O:
wz���F�Pp�i�T	}C<ba�.$��W��`��&�q����i�]-e&g��/��U�
���v%
����|sPF��ِ���S�m�8ݶ�GTʽT8eH�b�Q�B��h��!s�$�qlC��7�$aզ�|�N�Od�;���m0�ip��3+KV�{�ú)�f���tS�ޮ~���4L���?[��+0�O�ۆ��8J�I���X�m�ց�J����;�&���v�$���\����W �鹫�"�3T��ʟk�4Wn���T�@��9����p-�HX臲4�A���{�?�fC�ͥ�0f�붲b�y z'�Se��%�j}��,4b�v��%��ٗP}�c��
T���Q)�����ဟ�s���#E�?��*���8��Ȃ!H=&�퐨[���|�s��ͮF|E�/>^�a�[��r�Q?i�O�ykQl�-����-���+�W�*��'�N�=�a�7�TmiU/C�ڌ�=b��n]h	1}��1�3�0��A�խ^�����ӿq�n1���}�!ĝ�<�����CzQ���+�c����yFx�,�+�b��J���V� 'q:�DR���AdH�~I���9_�&~~x����UƯ:Ŏ���{�hU[��k��MV� Y�:�斶5W^�:���O?lA{\D��)A��_���GC�-�Z3%.0�d�R}�F���I�6f6#Ũ,�@ѕ�����7���x�1%��� BK�p���ر�
��#n_m6�;͇��9�e���Tʜ�'a�v�T��a��=s�ۈK�=c�ʹ�Lq�K]
:Ea�\��Y����i�HY���-m�k�*�V`���^��أq �N�^�&q�e����O�j$��"�n`O{@a���REn��'�qw����ۃ �S���#�f��5�3b��a��$�|܌��E|��٢�
��}�~�)v}0����D�t������T�vaA�;�~��M2�h~�DƳN�y�%�R(��ȧ����Z���%���9��%�{��Rԗ���m"����q�<h����#�@*M��q�E9����S3c��9�k���w��J��Q� �Ԕ�^�ZP��T����B��
$����l$��ImQ@��+�xG��&J��ҥ�є��I�+R��Lt�2B�a&�[Y����|��Ҽf�I�� D�H�`�8�Mr��8�}ᅂ��e�4X ��y�(����RϮ�?Yj%|�cI��P%X�s2�>�z��[zK�7+�W���uj�o��ȧ�kP 
c�7Ȳ�%w)ʐ�	����i@��NbU�����3�H
���6(���M��;��̌���5~D�q�ɷ��ݑ����� ԝ�B�������&�/�����&nK�����rSߗ98�ǯ��]L���1{���ư0%�	خ�:$����F�Y�T ��K&Z���zS��>W������^�U��l�m� �a������ �<֕��J\�r�ٲ\&�V��^*�Q+A�s�� k�Aq�0$�P*93���p~�o':�Ý@��'�-�}�X���2�)b�wM���"�Z3A�`�c邖�kʚ��f�'��erc#弨�x8���dS�{��K��7�*�<���V�XP�#���3�t9��A�d�FZ)-X�|�5�&أG7�����o1d7�����\��j���/����\�*�./𱗷mB@?r1��FRr�|A%��V�E�8W�1�|�ь6[v;��xe|"5T��vy�C�Ӊ�Y��f�\����ȵ8Y�4����y�+�Rw��y>'�sϞJe������~L`�L��u��1��.�L��mμ���Dz,)+I>bo�,m���	�
��/����tÿ�$���|��䘇c慣�Dls8�[aQ�W���ѾHA�f|n�L�I���.)�CĶsQ.������K}��F�b�B�t`:dӑC
s�W/VQ�0�Ggd4nz%�QdR��a ̲����𽃮ˍ�"Ҧa�;��(�MX�I�?�@�O�>�oy\����ƫf�Tֶ��ڽ�g�|e�wlC�p��Y����2L�C�ղf��3��:9�,���Y)��E�Io]g91D��KМ�w��-Qlw��J�w;�uH�;a�1@]���?�|�ɳ4�;xF7HZ}O��<��	��1݃|J%��#ڨɣ1�i�!g�y�B��q3��p����CQo,�O��ȥ^6eڃ����!L��z���WW8� NJ�]�Ũum*S@B}�c��g�ie|><�H(�ȔJk�ENO�F�ZCҜ�6'=g89��A�Ѫtk3F����ˉ���ĺڷ5拉gh1���PLX�*8�k�/�*:�6_ڪ�&�dH���[��Uz���a�8K@c� �m��ݽH�U��
�ECN^�R���;�+�Gs�^v3��
=a�w�X���j�C�%���[�&��J&��{��D�W�_��
�1��� >�e�>7;{U85�7�rQ�;h6�|�<q���} e��L�X��Yc���$�c�@�:gdd�1������W���-�����6��Q�����(��i�^c���ưQ�Yn��k6���w�Z
ټ��T����K�qq5�5/��<hȁun}B<�I�`�9�NQ�Z]�p>V�m�R9����/���� �7-�������*���}b���ǈ)��g�_�0���6�Z-vy(����d�\�,����[{L+۩�-o�7vo
[���S�;:�'�
w���$r�ڲ�	�����}�&pRBL�K�A�ܢ�!n,
*9��=���A��۸
�l̮0�������%~��mV�΀�	��R����
W�"�H�=	�Jf�h`*bȸŉq����梦5���ŗ�j?��ȰdB����&�Ο�R�\�e��$P^�1�G��&uo*�����OcZ�=�8S�D�S�m�>_V�Sl-Ϊ�nH!�Pb�%wn��(ې>+�G��.��ä'= ��u��С�^&����T,DL�l+ۖ��)"딌��a�\߄��a%㱉�KA�t6�F���E~X��q~ha���|�k�n�a���-�"�Ih�{�G�n)���@�����O����9�h,;p��m*��-of�u����*]��W�/"��f����)+èx^+����0��f�	�$"*P��8'	氼�+U���^-�}F6s�{�d�\��̸�=��b71�wCq��!9�� ��dz�v7kQ�7R�R���?�^�}��\�U����I������
�a�N�i��3fz�R*���'�U���h��wJ�%rDy�
3� ���{>�q�.��,z��2.hp���<.v�3
�:�K�*s��������,`��ϓL�ϑ��P^= 6:�bl`1�b!'�I�,�S
��%����D�����'�zF�Ϧ�iT_}�ef<�F��*& 6�*�H�3�ƕS��>8�a�"�_:q8N�8���/�*=#�f����*���3����篔��Q
D�U{R������|����L��Q8<D�Q�p*�:��e;�l��`���*k<�
�tXě��A��V��D7�V3��%�u&1r,��tYB�9�����{?����(0�?=����pF��53b���ؓU�;���̞�TN(��6,�����:���`Pع+0�Ɠ���A߈��2D�46����A@���<��n�7�'���t~_hSG�E$"����j�f�6Ͻ�yUe�K{X�hc�5���!\0�!,k[���w�=s�����;��+������)�1�]d@k�D?��KS�����R����O�9x}�py:@�:�4A�H9�#b�9�N9J��-g
��3��x�
��O��q��e~�L���R������S����GA[�o�NX�E>!��"���	J��~Y����>,�2�m��$��1	�c���,H�o	5������]UtgCT�J�0-��Q�lr�_���P<�v�l��=��B673O]��A��]9h� <�O�ڡNT��M~�%�
�>��oZ���N0�%`��'$!{i�1���J<ug��?�ltG�B4�~�u��Z�pi�Ӏ���=�r�ʋ��K)
e�4zI�״ٶ������-w������;ނ>8����'�b�_����Ƕ]	�K���.02�;�.���$�*`��v�ErDʦ�A��{C�Eu+�4-llD-x��"��e�a���̑#��_P%�E��{�9y���8n}I<�b������:~�͡�(.?��}��HF3����{��7�@_�6�T���V��!�8����b7w[���Ht�
��d
�JY��߳RU%���"�/��m�K���^��u�o�G��nGƭ����3m>z�F�:Uy��gN��+I��c��-�ҧ6�w�����m���