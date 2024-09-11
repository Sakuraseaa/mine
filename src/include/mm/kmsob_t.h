#ifndef _KMSOB_T_H_
#define _KMSOB_T_H_

#define MSCLST_MAX (5)
#define KOBLST_MAX (64)
#define KUC_NEWFLG (1)
#define KUC_DELFLG (2)
#define KUC_DSYFLG (3)

//�����ڴ��������ռ�õ��ڴ�ҳ������Ӧ��msadsc_t�ṹ
typedef struct s_MSCLST
{
    uint_t ml_msanr;  //���ٸ�msadsc_t
    uint_t ml_ompnr;  //һ��msadsc_t��Ӧ�������������ڴ�ҳ����
    list_h_t ml_list; //����msadsc_t������
}msclst_t;


//�����ڴ��������ռ�õ��ڴ�
typedef struct s_MSOMDC
{
    //msclst_t�ṹ����mc_lst[0]=1������ҳ���msadsc_t
    //               mc_lst[1]=2������ҳ���msadsc_t
    //               mc_lst[2]=4������ҳ���msadsc_t
    //               mc_lst[3]=8������ҳ���msadsc_t
    //               mc_lst[4]=16������ҳ���msadsc_t
    msclst_t mc_lst[MSCLST_MAX];
    uint_t mc_msanr;   //�ܹ����msadsc_t�ṹ
    list_h_t mc_list;
    //�ڴ����������һ��ռ��msadsc_t
    list_h_t mc_kmobinlst;
    //�ڴ����������һ��ռ��msadsc_t��Ӧ�������������ڴ�ҳ����
    uint_t mc_kmobinpnr;
}msomdc_t;

// �ڴ����
typedef struct s_FREOBJH
{
	list_h_t oh_list; // ����
	uint_t oh_stus;	 // ����״̬
	void* oh_stat;	// ����Ŀ�ʼ��ַ
}freobjh_t; // free object head

//�ڴ��������
typedef struct s_KMSOB
{
    list_h_t so_list;        //����
    spinlock_t so_lock;      //�����ṹ�����������
    uint_t so_stus;          //״̬���־
    uint_t so_flgs;
    adr_t so_vstat;          //�ڴ���������Ŀ�ʼ��ַ
    adr_t so_vend;           //�ڴ���������Ľ�����ַ
    size_t so_objsz;         //�ڴ�����С
    size_t so_objrelsz;      //�ڴ����ʵ�ʴ�С
    uint_t so_mobjnr;        //�ڴ�����������ܹ��Ķ������
    uint_t so_fobjnr;        //�ڴ���������п��еĶ������
    list_h_t so_frelst;      //�ڴ���������п��еĶ�������ͷ
    list_h_t so_alclst;      //�ڴ���������з���Ķ�������ͷ
    list_h_t so_mextlst;     //�ڴ����������չkmbext_t�ṹ����ͷ
    uint_t so_mextnr;        //�ڴ����������չkmbext_t�ṹ����
    msomdc_t so_mc;          //�ڴ��������ռ���ڴ�ҳ�����ṹ
    void* so_privp;          //���ṹ˽������ָ��
    void* so_extdp;          //���ṹ��չ����ָ��
}kmsob_t;

// �����ڴ����������չ����
typedef struct s_KMBEXT
{
	list_h_t mt_list; 	// ����
	adr_t mt_vstat;		// �ڴ����������չ������ʼ��ַ
	adr_t mt_vend;		// �ڴ����������չ����������ַ
	kmsob_t* mt_kmsb;	// ָ���ڴ���������ṹ
	uint_t mt_mobjnr;	// �ڴ����������չ�������ڴ����ж��ٶ���
}kmbext_t;


// ����kmsob_t�ڴ�ֽṹ, �ڱ�ϵͳ��boklstͳ�� ����(a, a +31)��С���ڴ������ڴ��
// �����˵ boklst ������32���ڴ�أ�ÿһ���ڴ���������гߴ���ڴ����
typedef struct s_KOBLST
{
	list_h_t ol_emplst;	// ����kmsob_t�ṹ������
	kmsob_t* ol_cahe;	// ���һ�β��ҵ�kmsob_t�ṹ
	uint_t ol_emnr; // ����kmsob_t�ṹ������, �����ڴ�ص�����
	size_t ol_sz;	// kmsob_t�ṹ���ڴ����Ĵ�С
}koblst_t;

// ����kmsob_t�ṹ�����ݽṹ
typedef struct s_KMSOBMGRHED
{
	spinlock_t ks_lock;
	list_h_t ks_tclst;
	uint_t ks_tcnr;
	uint_t ks_msobnr;	// �ܹ����ٸ�kmsob_t�ṹ
	kmsob_t* ks_msobche;	// ��������ڴ�����kmsob_t�ṹ
	koblst_t ks_msoblst[KOBLST_MAX];
}kmsobmgrhed_t; // kernel memory space object manager head

typedef struct s_KOBCKS
{
	list_h_t kk_list;
	void* kk_vadr;
	size_t kk_sz;
}kobcks_t;

void init_kmsob();
void *kmsob_new(size_t msz);
#endif // _KMSOB_T_H_