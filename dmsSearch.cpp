
extern "C"
{
#include "edbtypes.h"
#include "ecs_meastype.h"
#include "edb_ext.h"
#include "scadadb.h"

#include "pasMsg.h"
#include "pasdbtpar.h"
#include "pasdbtdef.h"
#include "pasdb.h"
#include "pasdbt.h"
}

#include "psctree.h"

#include "dmsSearch.h"

//���ڱ����ȡ���ȱ����и�Ԫ���ı���״̬
#define TRAVEL_PENDENT 0  //δ�����
#define TRAVEL_VISITED 1  //����(����)��
#define TRAVEL_TOVISIT 2  //��ѹ������ʶ�ջ

int CDmsSearch::m_siFilterLeafNum = 0;

CDmsSearch::CDmsSearch()
{
    m_pTeVisitFlagRcd = NULL;
    m_pEqVisitFlagRcd = NULL;
    m_pToVisitstack   = NULL;
    m_pEqSearchLevel  = NULL;

    m_iToVisitNum = 0;
    m_iFeederId   = -1;
}

CDmsSearch::~CDmsSearch()
{
    if( m_pTeVisitFlagRcd != NULL )
    {
        delete[] m_pTeVisitFlagRcd;
        m_pTeVisitFlagRcd = NULL;
    }

    if( m_pEqVisitFlagRcd != NULL )
    {
        delete[] m_pEqVisitFlagRcd;
        m_pEqVisitFlagRcd = NULL;
    }

    if( m_pToVisitstack != NULL )
    {
        delete[] m_pToVisitstack;
        m_pToVisitstack = NULL;
    }

    if( m_pEqSearchLevel != NULL )
    {
        delete[] m_pEqSearchLevel;
        m_pEqSearchLevel = NULL;
    }
}

/********************************************************************************************
���ܣ��жϸ��������Ƿ��豸����Դ����ӡ�
������
    ����: iSrcIdx����������������
    ���: �ޡ�
����: ����Դ�㷵��true, ���򷵻�false��
˵����1.����������������������������ѹ����ڿ��ػ�բ
     2.������ͨ�������Ƿ�����ҵ�ĸ�ߣ��ж��Ƿ�Ϊ����Դ�ࡣ
*********************************************************************************************/
#define PAS_SEARCH_TE_MAX_LEVEL 100
bool CDmsSearch::SearchNoSwEqFromTe( int iSrcTeIdx, int& iType, int& iTypeIdx )
{
    int iObjType                             = 0;
    int iAryTeLevel[PAS_SEARCH_TE_MAX_LEVEL] = { 0 };
    int iLevel                               = 0;
    int iFlag                                = 0;
    int iCount                               = 0;
    int iLoopNum                             = 0;

    sint32 iCurTe  = 0;
    sint32 iFromTe = 0;
    sint32 iToTe   = 0;
    sint32 iNd     = 0;
    sint32 iBsIdx  = -1;
    sint32 iSwIdx  = 0;
    sint32 iRbIdx  = 0;

    if( PAS_IS_INVALID( Te, iSrcTeIdx ) )
        return false;

    iNd = PAS_TeSta[iSrcTeIdx].nd;
    if( PAS_IS_INVALID( Nd, iNd ) )
        return false;

    int* pTeFlag = new int[PAS_TeUsedCat->extent + 1];
    int* pNdFlag = new int[PAS_NdUsedCat->extent + 1];
    memset( pTeFlag, 0, sizeof( int ) * ( PAS_TeUsedCat->extent + 1 ) );
    memset( pNdFlag, 0, sizeof( int ) * ( PAS_NdUsedCat->extent + 1 ) );
    memset( iAryTeLevel, 0, sizeof( int ) * PAS_SEARCH_TE_MAX_LEVEL );

    pTeFlag[iSrcTeIdx] = 1;
    iLevel             = 0;
    iCount             = 0;
    while( 1 )
    {
        if( pNdFlag[iNd] == 0 )  //δ���ʹ���nd��ջ
        {
            if( iLevel >= PAS_SEARCH_TE_MAX_LEVEL - 1 )
                break;

            iLevel++;
            iAryTeLevel[iLevel] = iNd;
            pNdFlag[iNd]        = 1;
        }

        iCount++;
        if( iCount > 1000 )
        {
            printf( "SearchFromTe COUNT OVER: cur_count = %d, max_count = %d\n", iCount, 1000 );
            break;
        }

        iLoopNum = 0;
        for( iCurTe = PAS_NdSta[iNd].firstTe; PAS_IS_VALID( Te, iCurTe ); iCurTe = PAS_TeSta[iCurTe].nextTe )  //�ҳ���ǰ�ڵ�ķǿ����豸�����ж�
        {
            iLoopNum++;
            if( iLoopNum > 999 )
                break;

            iObjType = PAS_TeSta[iCurTe].objType;
            if( iObjType != PAS_TYP_SW )
            {
                if( iObjType == PAS_TYP_RB )  //�Ե翹������
                {
                    iRbIdx = PAS_TeSta[iCurTe].obj;
                    if( PAS_IS_VALID( Rb, iRbIdx ) && PAS_RbSta[iRbIdx].type == PAS_EQ_CON_TYPE_SER )  //�����翹��������
                        continue;
                    else
                        break;
                }
                else
                {
                    if( iCurTe == iSrcTeIdx || pTeFlag[iCurTe] == 1 )
                        continue;
                    else
                        break;
                }
            }
        }

        if( iObjType == PAS_TYP_BS && PAS_IS_VALID( Te, iCurTe ) )  //����ĸ�ߣ�˵���ǽ���Դ��
        {
            iType    = PAS_TYP_BS;
            iTypeIdx = PAS_TeSta[iCurTe].obj;
            return true;
        }
        else if( ( iObjType == PAS_TYP_LN && PAS_IS_VALID( Te, iCurTe ) ) )  //������·��˵����Զ��Դ��
        {
            iType    = PAS_TYP_LN;
            iTypeIdx = PAS_TeSta[iCurTe].obj;
            return true;
        }

        iFlag    = 0;
        iLoopNum = 0;
        for( iCurTe = PAS_NdSta[iNd].firstTe; PAS_IS_VALID( Te, iCurTe ); iCurTe = PAS_TeSta[iCurTe].nextTe )  //�ҵ���Nd��û�ѹ���Te
        {
            iLoopNum++;
            if( iLoopNum > 999 )
                break;

            if( pTeFlag[iCurTe] == 0 )
            {
                pTeFlag[iCurTe] = 1;
                iFlag++;
                break;
            }
        }

        if( iFlag == 0 )  //��Nd��Te�����ѹ��ˣ���ջ������һ���������
        {
            iLevel--;
            if( iLevel <= 0 )
                break;

            iNd = iAryTeLevel[iLevel];
            continue;
        }

        if( PAS_IS_VALID( Te, iCurTe ) )  //�ӵ�ǰiCurTeͨ�����ػ����翹�ҳ���һ��Ҫ������nd
        {
            if( PAS_TeSta[iCurTe].objType == PAS_TYP_SW )
            {
                iSwIdx = PAS_TeSta[iCurTe].obj;
                if( PAS_IS_VALID( Sw, iSwIdx ) )
                {
                    iFromTe = PAS_SwSta[iSwIdx].fromTe;
                    iToTe   = PAS_SwSta[iSwIdx].toTe;
                    if( iFromTe == iCurTe )
                    {
                        iNd            = PAS_TeSta[iToTe].nd;
                        pTeFlag[iToTe] = 1;
                    }
                    else if( iToTe == iCurTe )
                    {
                        iNd              = PAS_TeSta[iFromTe].nd;
                        pTeFlag[iFromTe] = 1;
                    }
                }
            }
            else if( PAS_TeSta[iCurTe].objType == PAS_TYP_RB )
            {
                iRbIdx = PAS_TeSta[iCurTe].obj;
                if( PAS_IS_VALID( Rb, iRbIdx ) && PAS_RbSta[iRbIdx].type == PAS_EQ_CON_TYPE_SER )
                {
                    iFromTe = PAS_RbSta[iRbIdx].fromTe;
                    iToTe   = PAS_RbSta[iRbIdx].toTe;
                    if( iFromTe == iCurTe )
                    {
                        iNd            = PAS_TeSta[iToTe].nd;
                        pTeFlag[iToTe] = 1;
                    }
                    else if( iToTe == iCurTe )
                    {
                        iNd              = PAS_TeSta[iFromTe].nd;
                        pTeFlag[iFromTe] = 1;
                    }
                }
            }
        }

        if( PAS_IS_INVALID( Nd, iNd ) )  //�����һ��û��nd����ȡ������nd��������������δ���ʹ���te
        {
            iNd = iAryTeLevel[iLevel];
        }
    }

    if( pTeFlag )
    {
        delete[] pTeFlag;
        pTeFlag = NULL;
    }

    if( pNdFlag )
    {
        delete[] pNdFlag;
        pNdFlag = NULL;
    }

    return false;
}

/********************************************************************************************
���ܣ�������·����ȷ����������ID��
������
    ����: iType�������豸����; iTypeIdx-�����豸������
    ���: �ޡ�
���أ�ȷ�����߳ɹ�����true, ���򷵻�false��
˵�����ޡ�
*********************************************************************************************/
bool CDmsSearch::DetermineFeederIdByLnIdx( int iType, int iTypeIdx )
{
    if( iType != PAS_TYP_LN )
        return false;

    if( PAS_IS_INVALID( Ln, iTypeIdx ) )
        return false;

    int iEqIdx = PAS_LnSta[iTypeIdx].eq;
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    int iEqId = PAS_EqSta[iEqIdx].ID;
    if( iEqId <= 0 || iEqId > SCADA_DevidToIdxCat->extent )
        return false;

    uint32 uiDevIdx = SCADA_DevidToIdx[iEqId];
    if( uiDevIdx <= 0 || uiDevIdx > SCADA_DeviceCat->extent )
        return false;

    uint32 uiSubFeederIdx = SCADA_Device[uiDevIdx].subfeeder_idx;
    if( uiSubFeederIdx > 0 && uiSubFeederIdx <= SCADA_SubFeederCat->extent )
    {
        uint32 uiFdIdx = SCADA_SubFeeder[uiSubFeederIdx].feeder_idx;
        if( uiFdIdx > 0 && uiFdIdx <= SCADA_FeederCat->extent )
        {
            m_iFeederId = SCADA_Feeder[uiFdIdx].feederId;
            return true;
        }
    }

    return false;
}

/********************************************************************************************
���ܣ����ݸ����豸ȷ����ʼ�������ӡ�
������
    ����: iEqId�������豸ID��
    ���: iSearchTeIdx-��ʼ�������ӡ�
����: ȷ���ɹ�����ture, ���򷵻�false��
˵��: �ޡ�
*********************************************************************************************/
bool CDmsSearch::DetermineSearchTe( int iEqId, int& iSearchTeIdx )
{
    iSearchTeIdx = -1;

    if( iEqId <= 0 || iEqId > PAS_EqIDCat->extent )
        return -1;

    int iEqIdx = PAS_EqID[iEqId];
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    int iType    = PAS_EqSta[iEqIdx].type;
    int iTypeIdx = PAS_EqSta[iEqIdx].typeIdx;
    if( iType != PAS_TYP_SW )
    {
        printf( "Error: ֻ�ӿ��ص�բ��ʼ����!!!\n" );
        return false;
    }

    if( PAS_IS_INVALID( Sw, iTypeIdx ) )
        return false;

    int iFromTeIdx = PAS_SwSta[iTypeIdx].fromTe;
    int iToTeIdx   = PAS_SwSta[iTypeIdx].toTe;
    int iTargetType, iTargetTypeIdx;
    int iDmsSizeTeIdx = -1;

    SearchNoSwEqFromTe( iFromTeIdx, iTargetType, iTargetTypeIdx );
    if( iTargetType == PAS_TYP_LN )
    {
        iDmsSizeTeIdx = iFromTeIdx;
    }
    else
    {
        SearchNoSwEqFromTe( iToTeIdx, iTargetType, iTargetTypeIdx );
        if( iTargetType == PAS_TYP_LN )
        {
            iDmsSizeTeIdx = iToTeIdx;
        }
    }

    if( PAS_IS_INVALID( Te, iDmsSizeTeIdx ) )
    {
        printf( "Error: Ҫ�����豸(id:%d)�Ҳ��������豸���޷���������\n" );
        return false;
    }

    iSearchTeIdx = iDmsSizeTeIdx;
    DetermineFeederIdByLnIdx( iTargetType, iTargetTypeIdx );

    return true;
}

/********************************************************************************************
���ܣ����ݸ����豸���������Զ������ؽ�β�����ṹ��
������
    ����: iEqId�������豸ID��
    ���: pDmsTree-���ء���բ��ɵ����ṹ��
����: �ޡ�
˵��: �ޡ�
*********************************************************************************************/
void CDmsSearch::SearchAutoBkEndTree( int iEqId, CPscTree* pDmsTree )
{
    SearchDmsTree( iEqId, pDmsTree );

    FilterTreeNoAutoBkLeaf( *pDmsTree );
}

/********************************************************************************************
���ܣ����ݸ����豸�������ο��ص�բ�豸�������ṹ������
������
    ����: iEqId�������豸ID��
    ���: pDmsTree-���ء���բ��ɵ����ṹ��
����: �ޡ�
˵��: �ޡ�
*********************************************************************************************/
void CDmsSearch::SearchDmsTree( int iEqId, CPscTree* pDmsTree )
{
    int iSearchTeIdx = -1;
    if( !DetermineSearchTe( iEqId, iSearchTeIdx ) )
    {
        printf( "Error: �޷�ȷ��Ҫ�����豸(id:%d)�����������!!!\n" );
        return;
    }

    int iMaxTeSize = PAS_TeUsedCat->extent + 1;

    if( m_pToVisitstack != NULL )
    {
        delete[] m_pToVisitstack;
        m_pToVisitstack = NULL;
    }

    m_pToVisitstack = new int[iMaxTeSize];
    if( m_pToVisitstack == NULL )
        return;

    if( m_pTeVisitFlagRcd != NULL )
    {
        delete[] m_pTeVisitFlagRcd;
        m_pTeVisitFlagRcd = NULL;
    }

    m_pTeVisitFlagRcd = new int[iMaxTeSize];
    if( m_pTeVisitFlagRcd == NULL )
        return;
    memset( m_pTeVisitFlagRcd, 0, sizeof( int ) * iMaxTeSize );

    if( m_pEqVisitFlagRcd != NULL )
    {
        delete[] m_pEqVisitFlagRcd;
        m_pEqVisitFlagRcd = NULL;
    }

    int iMaxEqSize    = PAS_EqUsedCat->extent + 1;
    m_pEqVisitFlagRcd = new int[iMaxEqSize];
    if( m_pEqVisitFlagRcd == NULL )
        return;
    memset( m_pEqVisitFlagRcd, 0, sizeof( int ) * iMaxEqSize );

    if( m_pEqSearchLevel != NULL )
    {
        delete[] m_pEqSearchLevel;
        m_pEqSearchLevel = NULL;
    }

    m_pEqSearchLevel = new int[iMaxEqSize];
    if( m_pEqSearchLevel == NULL )
        return;
    memset( m_pEqSearchLevel, 0, sizeof( int ) * iMaxEqSize );

    pDmsTree->SetNodeJudgeFunc( NULL );

    int iObj   = PAS_TeSta[iSearchTeIdx].obj;
    int iEqIdx = PAS_SwSta[iObj].eq;
    if( pDmsTree->InsertNode( NULL, CTreeNodeData( PAS_EqSta[iEqIdx].ID ) ) == NULL )
        return;

    BFS_GetTeDownStreamTree( iSearchTeIdx, pDmsTree );
}

/********************************************************************************************
���ܣ���������õ�ָ���������εĿ��ص�բ�γɵ�����
������
    ���룺iSrcTeIdx����ʼ����λ��;
    �����pDmsTree����������������õ��������ݽṹ�����ڵ㰴�㼶���档
���أ�������false���ɹ�����true��
˵����
    1�����ù�������γɲ������������iSrcTeIdx�����л�����������л����Ļ���ͨ�������ݽṹ���ܱ�Ҫ����(��)ͼ����
    2�����ĸ��ڵ�Ϊ�գ����ڵ���ӽڵ㱣��Ĳ�����Ч�ߣ���Ϊһ�����ڵ�ֻ�ܶ�Ӧһ���ߣ�iSrcTeIdx�����ж��ֱ�����αߣ���
    3�������ǿ���״̬��
*********************************************************************************************/
bool CDmsSearch::BFS_GetTeDownStreamTree( int iSrcTeIdx, CPscTree* pDmsTree )
{
    TreeNode_PTR* pParentStack = new TreeNode_PTR[PAS_TeUsedCat->extent + 1];  //��¼�������ӵĸ����ڵ�
    if( pParentStack == NULL )
        return false;
    memset( pParentStack, 0, sizeof( TreeNode_PTR ) * ( PAS_TeUsedCat->extent + 1 ) );

    int step      = 0;
    m_iToVisitNum = 0;

    m_pToVisitstack[m_iToVisitNum] = iSrcTeIdx;
    pParentStack[iSrcTeIdx]        = pDmsTree->GetRoot();
    m_iToVisitNum++;

    //������ʼ���ӷ��ʱ�־
    int iFromTeIdx                = m_pToVisitstack[0];
    m_pTeVisitFlagRcd[iFromTeIdx] = TRAVEL_VISITED;

    step++;

    // int iNdIdx = PAS_TeSta[iFromTeIdx].nd;
    // if (PAS_IS_INVALID(Nd, iNdIdx))
    //     return false;

    bool       ret            = true;
    CTreeNode* pCurParentNode = NULL;
    for( int i = 0; i < m_iToVisitNum; i++ )
    {
        int iTeIdx     = m_pToVisitstack[i];
        pCurParentNode = pParentStack[iTeIdx];

        int iNdIdx = PAS_TeSta[iTeIdx].nd;
        if( PAS_IS_INVALID( Nd, iNdIdx ) )
            continue;

        int iCurTeEqIdx = GetEqIdxFromTe( iTeIdx );
        int iLevel      = m_pEqSearchLevel[iCurTeEqIdx];

        int iCurTeIdx = -1, iLoopNum = 0;
        for( iCurTeIdx = PAS_NdSta[iNdIdx].firstTe; PAS_IS_VALID( Te, iCurTeIdx ); iCurTeIdx = PAS_TeSta[iCurTeIdx].nextTe )
        {
            iLoopNum++;
            if( iLoopNum > 99 )
                break;

            if( m_pTeVisitFlagRcd[iCurTeIdx] == TRAVEL_VISITED )
                continue;

            int iEqIdx, iTe1, iTe2, iTe3;
            int sts = GetEqTeFromTe( iCurTeIdx, iEqIdx, iTe1, iTe2, iTe3 );
            if( sts <= 0 )
                continue;

            if( iCurTeIdx == iTe1 )
            {
                CTreeNode* pCurEqParent = pCurParentNode;
                InStack( iTe1, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe2, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe3, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
            }
            else if( iCurTeIdx == iTe2 )
            {
                CTreeNode* pCurEqParent = pCurParentNode;
                InStack( iTe2, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe1, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe3, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
            }
            else if( iCurTeIdx == iTe3 )
            {
                CTreeNode* pCurEqParent = pCurParentNode;
                InStack( iTe3, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe1, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
                InStack( iTe2, iEqIdx, pDmsTree, pCurEqParent, pParentStack, iLevel + 1 );
            }
            else
            {
                continue;
            }
        }
    }
}

/********************************************************************************************
���ܣ��Թ���������������豸��ջ������¼�����Ŀ��ء���բ�豸��
������
    ����: iTeIdx����������, iEqIdx-�豸����, pParentNode-��ջ���ӵ����еĸ��ڵ�, iLevel-��ջ�豸�ڹ�������еĲ㼶��
    ���: pDmsTree-�¼��뿪�ء���բ�ڵ�����ṹ, pParentStack-�����˶����������ڵ�ĸ��ڵ����顣
����: �ޡ�
˵��: ���¶��Ӵ�������ʶ�ջ���������ء���բ�豸����������������¶����������ڵ㡣
*********************************************************************************************/
void CDmsSearch::InStack( int iTeIdx, int iEqIdx, CPscTree* pDmsTree, CTreeNode*& pParentNode, TreeNode_PTR* pParentStack, int iLevel )
{
    if( pDmsTree == NULL || pParentStack == NULL )
        return;

    if( PAS_IS_INVALID( Te, iTeIdx ) )
        return;

    if( m_pTeVisitFlagRcd[iTeIdx] == TRAVEL_VISITED )
        return;

    CTreeNode* pNewAddNode = pParentNode;
    if( m_pEqSearchLevel[iEqIdx] == 0 )
    {
        m_pEqSearchLevel[iEqIdx] = iLevel;
    }

    if( PAS_EqSta[iEqIdx].type == PAS_TYP_BS )
    {
        if( IsSelfFeedBk( iEqIdx ) )
        {
            m_pToVisitstack[m_iToVisitNum] = iTeIdx;
            m_iToVisitNum++;
        }
    }
    else if( PAS_EqSta[iEqIdx].type == PAS_TYP_LN )
    {
        if( IsSelfFeedBk( iEqIdx ) )
        {
            m_pToVisitstack[m_iToVisitNum] = iTeIdx;
            m_iToVisitNum++;
        }
    }
    else if( PAS_EqSta[iEqIdx].type == PAS_TYP_SW )
    {
        if( m_pEqVisitFlagRcd[iEqIdx] != TRAVEL_VISITED )  //�Ѿ���¼��
        {
            pNewAddNode = pDmsTree->InsertNode( pParentNode, CTreeNodeData( PAS_EqSta[iEqIdx].ID ) );
            if( pNewAddNode == NULL )
            {
                abort();
            }

            printf( "level:%d ID:%d ���豸ID:%d\n", iLevel, PAS_EqSta[iEqIdx].ID, pParentNode->m_data.m_no );
        }

        if( IsAutoBk( iEqIdx ) )  //�Զ���������Ҫ��¼
        {
            if( IsSelfFeedBk( iEqIdx ) )  //�Զ��������ұ������ڵ���ջ
            {
                m_pToVisitstack[m_iToVisitNum] = iTeIdx;
                m_iToVisitNum++;
            }
        }
        else  //���Զ������ض���ջ
        {
            m_pToVisitstack[m_iToVisitNum] = iTeIdx;
            m_iToVisitNum++;
        }
    }

    pParentStack[iTeIdx]      = pNewAddNode;
    m_pTeVisitFlagRcd[iTeIdx] = TRAVEL_VISITED;
    m_pEqVisitFlagRcd[iEqIdx] = TRAVEL_VISITED;

    pParentNode = pNewAddNode;
}

/********************************************************************************************
���ܣ��ж��豸�Ƿ�����Ҫ�����ı����ߡ�
������
    ����: iEqIdx���豸������
    ���: �ޡ�
����: �Ǳ������ڵķ���true, ���Ƿ���false��
˵��: �ޡ�
*********************************************************************************************/
bool CDmsSearch::IsSelfFeedBk( int iEqIdx )
{
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    uint32 devId = PAS_EqSta[iEqIdx].ID;
    if( devId <= 0 || devId > SCADA_DevidToIdxCat->extent )
        return false;

    uint32 uiDevIdx = SCADA_DevidToIdx[devId];
    if( uiDevIdx <= 0 || uiDevIdx > SCADA_DeviceCat->extent )
        return false;

    uint32 uiSubFeederIdx = SCADA_Device[uiDevIdx].subfeeder_idx;
    if( uiSubFeederIdx > 0 && uiSubFeederIdx <= SCADA_SubFeederCat->extent )
    {
        uint32 uiFdIdx = SCADA_SubFeeder[uiSubFeederIdx].feeder_idx;
        if( uiFdIdx > 0 && uiFdIdx <= SCADA_FeederCat->extent )
        {
            if( SCADA_Feeder[uiFdIdx].feederId == m_iFeederId )
                return true;
            else
                return false;
        }
    }
    else  //�����Ե�����������
    {
        return true;
    }
}

/********************************************************************************************
���ܣ��ж��豸�Ƿ��Զ������ء�
������
    ����: iEqIdx���豸������
    ���: �ޡ�
����: ���Զ������صķ���true, ���Ƿ���false��
˵��: �ޡ�
*********************************************************************************************/
bool CDmsSearch::IsAutoBk( int iEqIdx )
{
    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return false;

    if( PAS_EqSta[iEqIdx].type != PAS_TYP_SW )
        return false;

    int iSwIdx = PAS_EqSta[iEqIdx].typeIdx;
    if( PAS_SwSta[iSwIdx].type >= PAS_SW_TYPE_PTONG )
        return false;

    int iEqId = PAS_EqSta[iEqIdx].ID;
    if( iEqId <= 0 || iEqId > SCADA_DevidToIdxCat->extent )
        return false;

    int iDevIdx = SCADA_DevidToIdx[iEqId];
    int iPtIdx  = SCADA_Device[iDevIdx].point_idx;
    if( SCADA_PointType[iPtIdx].point_class == ECS_K_PT_CLASS_AI ||
        SCADA_PointType[iPtIdx].point_class == ECS_K_PT_CLASS_DI )
        return true;
    else
        return false;
}

/********************************************************************************************
���ܣ���ȡָ�����ӵ��豸�����Լ��豸��ȫ�����ӡ�
������
    ����: iTeIdx��ָ������������
    ���: iEqIdx-�豸����, iTe1-����1, iTe2-����2, iTe3-����3��
����: ��ȡ���豸�Ķ���������
˵��: �ޡ�
*********************************************************************************************/
int CDmsSearch::GetEqTeFromTe( int iTeIdx, int& iEqIdx, int& iTe1, int& iTe2, int& iTe3 )
{
    if( PAS_IS_INVALID( Te, iTeIdx ) )
        return 0;

    iEqIdx = -1;
    iTe1   = -1;
    iTe2   = -1;
    iTe3   = -1;

    int iObjType = PAS_TeSta[iTeIdx].objType;
    int iObj     = PAS_TeSta[iTeIdx].obj;

    switch( iObjType )
    {
        case PAS_TYP_LN:
            if( PAS_IS_INVALID( Ln, iObj ) )
                break;

            iEqIdx = PAS_LnSta[iObj].eq;
            iTe1   = PAS_LnSta[iObj].fromTe;
            iTe2   = PAS_LnSta[iObj].toTe;
            break;
        case PAS_TYP_SW:
            if( PAS_IS_INVALID( Sw, iObj ) )
                break;

            iEqIdx = PAS_SwSta[iObj].eq;
            iTe1   = PAS_SwSta[iObj].fromTe;
            iTe2   = PAS_SwSta[iObj].toTe;
            break;
        case PAS_TYP_BS:
            if( PAS_IS_INVALID( Bs, iObj ) )
                break;

            iEqIdx = PAS_BsSta[iObj].eq;
            iTe1   = PAS_BsSta[iEqIdx].te;
            break;
        default:
            break;
    }

    int teNum = 0;
    if( PAS_IS_VALID( Te, iTe1 ) )
        teNum++;

    if( PAS_IS_VALID( Te, iTe2 ) )
        teNum++;

    if( PAS_IS_VALID( Te, iTe3 ) )
        teNum++;

    return teNum;
}

/********************************************************************************************
���ܣ���ȡ���ӵ��豸������
������
    ����: iTeIdx������������
    ���: �ޡ�
����: �����������豸������
˵��: �ޡ�
*********************************************************************************************/
int CDmsSearch::GetEqIdxFromTe( int iTeIdx )
{
    if( PAS_IS_INVALID( Te, iTeIdx ) )
        return 0;

    int iEqIdx = -1;

    int iObjType = PAS_TeSta[iTeIdx].objType;
    int iObj     = PAS_TeSta[iTeIdx].obj;

    switch( iObjType )
    {
        case PAS_TYP_LN:
            if( PAS_IS_INVALID( Ln, iObj ) )
                break;

            iEqIdx = PAS_LnSta[iObj].eq;
            break;
        case PAS_TYP_SW:
            if( PAS_IS_INVALID( Sw, iObj ) )
                break;

            iEqIdx = PAS_SwSta[iObj].eq;
            break;
        case PAS_TYP_BS:
            if( PAS_IS_INVALID( Bs, iObj ) )
                break;

            iEqIdx = PAS_BsSta[iObj].eq;
            break;
        default:
            break;
    }

    if( PAS_IS_INVALID( Eq, iEqIdx ) )
        return 0;

    return iEqIdx;
}

/********************************************************************************************
���ܣ��������ṹҶ�ӽڵ�Ļص�������
������
    ����: treeNodeData���ڵ�����, pUserObjData-�û����ݡ�
    ���: �ޡ�
����: �ޡ�
˵��: �ж�Ҷ�ӽڵ㱣����Ƿ��Զ������أ�������ɾ����
*********************************************************************************************/
void CDmsSearch::DisposeDmsTreeNode( CTreeNodeData& treeNodeData, void* pUserObjData )
{
    if( pUserObjData == NULL )
        return;

    CDisposeTreeUserData* pDisposeData = (CDisposeTreeUserData*)pUserObjData;

    CPscTree*   pDmsTree   = pDisposeData->m_pDmsTree;
    CDmsSearch* pDmsSearch = pDisposeData->m_pDmsSearch;

    int iEqId  = treeNodeData.m_no;
    int iEqIdx = PAS_EqID[iEqId];

    if( !pDmsSearch->IsAutoBk( iEqIdx ) )
    {
        //printf( "ɾ���ǲɼ�����Ҷ�ӽڵ�: %d\n", iEqId );
        pDmsTree->DeleteNode( NULL, treeNodeData );
        m_siFilterLeafNum++;
    }
}

/********************************************************************************************
���ܣ��������ṹ�еķ��Զ�������Ҷ�ӽڵ㡣
������
    ����: rDmsTree��Ҫ��������ṹ��
    ���: �ޡ�
����: �ޡ�
˵��: �����ṹ�е�Ҷ�ӽڵ����ѭ��������ÿ�α���ʱ�Ѳ����Զ������ص�Ҷ�ӽڵ�ȫ��ɾ����ֱ�����е�Ҷ�ӽڵ㶼���Զ������ء�
*********************************************************************************************/
void CDmsSearch::FilterTreeNoAutoBkLeaf( CPscTree& rDmsTree )
{
    CDisposeTreeUserData rDisposeData( &rDmsTree, this );

    rDmsTree.RegisterUserData( (void*)( &rDisposeData ) );
    rDmsTree.PostOrderTraverseLeaf( DisposeDmsTreeNode );

    while( m_siFilterLeafNum > 0 )
    {
        m_siFilterLeafNum = 0;
        rDmsTree.PostOrderTraverseLeaf( DisposeDmsTreeNode );
    }

    rDmsTree.UnRegisterUserData();
}

/********************************************************************************************
���ܣ����ָ������ģ�͡�
������
    ����: rDmsTree��Ҫ��������ṹ��
    ���: �ޡ�
����: �ޡ�
˵��: �ޡ�
*********************************************************************************************/
void CDmsSearch::OutTreeModel( CPscTree& rDmsTree )
{
    printf( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    printf( "<Feeder Id=\"%d\">", m_iFeederId );
    printf( "    <DeviceInfo>\n" );

    DFS_TravelOutTree( rDmsTree, rDmsTree.GetRoot() );

    printf( "    </DeviceInfo>\n" );
    printf( "</Feeder>\n" );
}

/********************************************************************************************
���ܣ���ȵݹ��������ģ�ͽṹ��
������
    ����: rDmsTree��Ҫ��������ṹ, pCurNode-��ǰ��������ڵ㡣
    ���: �ޡ�
����: �ޡ�
˵��: �ޡ�
*********************************************************************************************/
void CDmsSearch::DFS_TravelOutTree( CPscTree& rDmsTree, CTreeNode* pCurNode )
{
    printf( "        <Device Id=\"%d\"", GetEqIdOfTreeNode( pCurNode ) );
    // int nCurNodeChildNum = rDmsTree.GetChildCount( pCurNode );
    // if( nCurNodeChildNum > 0 )
    // {
    //     //printf( "LinkNum=%d ", nCurNodeChildNum );
    //     //printf( " LinkInfo=\"" );
    // }

    CTreeNode* pChildNode = pCurNode->GetFirstChild();

    int iEqId = 0;
    while( pChildNode != NULL )
    {
        if( iEqId > 0 )
        {
            printf( ";" );
        }
        else
        {
            printf( " LinkInfo=\"" );
        }

        iEqId = GetEqIdOfTreeNode( pChildNode );
        printf( "%d", iEqId );

        pChildNode = pChildNode->GetNextNeighbor();
    }  //while

    if( iEqId > 0 )
        printf( "\"/>\n" );
    else
        printf( "/>\n" );

    pChildNode = pCurNode->GetFirstChild();
    while( pChildNode != NULL )
    {
        DFS_TravelOutTree( rDmsTree, pChildNode );

        pChildNode = pChildNode->GetNextNeighbor();
    }  //while
}

/********************************************************************************************
���ܣ������ڵ��ȡ�豸ID��
������
    ����: pCurNode-��ǰ��������ڵ㡣
    ���: �ޡ�
����: �ɹ������豸ID�����򷵻�-1��
˵��: �ޡ�
*********************************************************************************************/
int CDmsSearch::GetEqIdOfTreeNode( CTreeNode* pCurNode )
{
    if( pCurNode == NULL )
    {
        return -1;
    }

    CTreeNodeData& rNodeData = pCurNode->m_data;
    return rNodeData.m_no;
}
