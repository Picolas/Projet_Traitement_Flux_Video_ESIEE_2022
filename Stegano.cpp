#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>


typedef struct
{
	uchar bit7 : 1;
	uchar bit6 : 1;
	uchar bit5 : 1;
	uchar bit4 : 1;
	uchar bit3 : 1;
	uchar bit2 : 1;
	uchar bit1 : 1;
	uchar bit0 : 1;
}st_bits;

st_bits convert(uchar data) {
	
	st_bits retour;
	for (int i = 7; i >= 0; i--) {
		if (data > pow(2, i)) {
			if (7 == i) retour.bit7 = 1;
			if (6 == i) retour.bit6 = 1;
			if (5 == i) retour.bit5 = 1;
			if (4 == i) retour.bit4 = 1;
			if (3 == i) retour.bit3 = 1;
			if (2 == i) retour.bit2 = 1;
			if (1 == i) retour.bit1 = 1;
			if (0 == i) retour.bit0 = 1;
			data -= (int) pow(2, i);
		}
		else {
			if (7 == i) retour.bit7 = 0;
			if (6 == i) retour.bit6 = 0;
			if (5 == i) retour.bit5 = 0;
			if (4 == i) retour.bit4 = 0;
			if (3 == i) retour.bit3 = 0;
			if (2 == i) retour.bit2 = 0;
			if (1 == i) retour.bit1 = 0;
			if (0 == i) retour.bit0 = 0;
		}
	}
	return retour;

}


// Convertie une valeur d�cimale en bit ( stock� dans un tableau de int[8] )
void decToBit(int dec, int* bits) {

	int temp = dec;
	for (int i = 7; i >= 0; i--)
	{
		// test si la valeur decimale est inf�rieur � 2^i
		if (dec < pow(2, i)) {
			bits[i] = 0;
		}
		else {
			// si la valeur moins 2^i est sup�rieur � 0 alors on peut le retirer -> 1 � cet emplacement
			if ((dec - pow(2, i)) >= 0) {
				bits[i] = 1;
				dec = dec - pow(2, i);
			}
			// sinon 0
			else {
				bits[i] = 0;
			}
		}
	}

}

// Convertie un tableau de bit ( int[8] ) en une valeur d�cimale
int bitToDec(int* bits) {

	int result = 0;

	// Pour chaque bit
	for (int i = 0; i < 8; i++)
	{
		// ajoute sa valeur en puissace de 2^i si elle est �gale � 1
		if (bits[i] == 1) result += (int) pow(2, i);
	}

	return result;
}

// Calcul le hash pour effectuer la st�ganographie
int hash(int posHiddenbit, int nbLSB = 4) {

	return posHiddenbit % nbLSB;

}

// Insertion des bit dans les bon emplacement via la caclul d'un hash
void insertBitWithHash(int* bits, int* Bbit, int* Gbit, int* Rbit) {

	// Test si le bit � ins�rer est :
	for (int i = 0; i < 8; i++)
	{
		// dans la couleur Rouge
		if (i >= 0 && i < 3) {
			int h = hash(i);
			for (int j = 0; j < 8; j++)
			{
				if (j == h) {
					Rbit[j] = bits[i];
				}
			}
		}
		// dans la couleur Bleu ( Green )
		if (i >= 3 && i < 6) {
			int h = hash(i);
			for (int j = 0; j < 8; j++)
			{
				if (j == h) {
					Gbit[j] = bits[i];
				}
			}
		}
		// dans la couleur Bleu
		if (i >= 6 && i < 8) {
			int h = hash(i);
			for (int j = 0; j < 8; j++)
			{
				if (j == h) {
					Bbit[j] = bits[i];
				}
			}
		}
	}

}

// Copie un tableau dans un autre
void tabCopy(int* out, int* in) {

	for (int i = 0; i < 8; i++)
	{
		out[i] = in[i];
	}

}

void steganoAt(int i, int j, int value, cv::Mat& img, cv::Mat& result) {

	// insertion du nombre de lignes
	int bits[8];
	decToBit(value, bits);
	int ValBGR[3];
	ValBGR[0] = img.at<cv::Vec3b>(i, j)[0];
	ValBGR[1] = img.at<cv::Vec3b>(i, j)[1];
	ValBGR[2] = img.at<cv::Vec3b>(i, j)[2];

	// R�cup�ration des bits de l'image source
	int B[8], G[8], R[8];
	decToBit(ValBGR[0], B);
	decToBit(ValBGR[1], G);
	decToBit(ValBGR[2], R);

	// Copy dans le tableau finale
	int Bbit[8], Gbit[8], Rbit[8];
	tabCopy(Bbit, B);
	tabCopy(Gbit, G);
	tabCopy(Rbit, R);

	// insertion de la valeur de row dans les bits de couleur de lena
	insertBitWithHash(bits, Bbit, Gbit, Rbit);

	// Transformation de bits en d�cimal et affectation de la valeur � chaque couleur
	int newB = bitToDec(Bbit);
	result.at<cv::Vec3b>(i, j)[0] = newB;

	int newG = bitToDec(Gbit);
	result.at<cv::Vec3b>(i, j)[1] = newG;

	int newR = bitToDec(Rbit);
	result.at<cv::Vec3b>(i, j)[2] = newR;

}



int main(int argc, char* argv[]) {


	if (argc < 3) {
		std::cout << "erreur dans l'appel de l'executable !!" << std::endl;
		return 0;
	}
	// Cr�ation des matrices contenant chaque image utilis� pour ce programme
	cv::Mat img1 = cv::imread(argv[1]);
	cv::Mat img2 = cv::imread(argv[2]);
	cv::Mat img2_gray;
	cv::Mat stegano;

	// copie de lena dans le resultat -> initialise la taille de resultat
	img1.copyTo(stegano);

	// cr�e l'image en niveau de gris � ins�rer � partir de l'image 2
	cv::cvtColor(img2, img2_gray, cv::COLOR_BGR2GRAY);

	std::cout << "row gray : " << img2_gray.rows << " et col gray : " << img2_gray.cols << std::endl;

	// Insertion des valeurs de Mat de l'image 2 dans image 1

	// cr�ation du offset pour ins�rer l'image � la suite
	// avec un offset par lignes/colonnes qui permet de d�passer la limite de 255 donner par un pixel
	int temp = 0;
	int offset = 4;
	int rowoffset = 1;
	int coloffset = 1;

	// r�cup�ration des lignes et des colonnes
	int row = img2.rows;
	int col = img2.cols;

	// calcul du nombre d'offset pour les lignes
	temp = row;
	while (temp > 255) {
		offset += 1;
		temp -= 255;
		rowoffset += 1;
	}

	// calcul du nombre d'offset pour les colonnes
	temp = col;
	while (temp > 255) {
		offset += 1;
		temp -= 255;
		coloffset += 1;
	}

	// Ajout des valeurs pour la quantit� de lignes et de colonnes
	steganoAt(0, 0, rowoffset, img1, stegano);
	steganoAt(0, 1, coloffset, img1, stegano);

	// Ajout de la valeur pour les lignes
	temp = row;
	for (int i = 2; i < 2 + rowoffset; i++)
	{
		if (temp > 255) {
			steganoAt(0, i, 255, img1, stegano);
			temp -= 255;
		}
		else {
			if(temp > 0) steganoAt(0, i, temp, img1, stegano);
		}
	}

	// Ajout de la valeur pour les colonnes
	temp = col;
	for (int i = 2 + rowoffset; i < 2 + coloffset + rowoffset; i++)
	{
		if (temp > 255) {
			steganoAt(0, i, 255, img1, stegano);
			temp -= 255;
		}
		else {
			if(temp > 0) steganoAt(0, i, temp, img1, stegano);
		}
	}

	// Insertion de l'image 2 dans 1
	
	// It�ration pour chaque lignes
	for (int i = 0; i < row; i++) {
		// It�rations pour chaque colonnes
		for (int j = offset; j < col + offset; j++) {

			// r�cup la valeur d�cimal du gray pour chaque pixel
			int val_img2 = img2_gray.at<cv::uint8_t>(i, j - offset);

			// effectue la st�gano en appelant avec les bon parama�tres la fonction suivante

			// if i == 0, pas de offset car les valeurs doivent �tre d�cal� pour pas �craser les valeurs de Mat rentrer pr�cedemment
			if (0 == i) steganoAt(i, j, val_img2, img1, stegano);
			// else, offset pour remmetre les valeurs � partir du d�but de l'image
			else steganoAt(i, j - offset, val_img2, img1, stegano);

		}
	}

	// affichage de chaque image ainsi que l'image finale
	cv::imshow("image 1", img1);
	cv::imshow("image 2", img2);
	cv::imshow("img2_gray", img2_gray);
	cv::imshow("result", stegano);

	// sauvegarde de l'image sur le stockage de l'ordinateur
	cv::imwrite("stegano.png", stegano);

	// delai afin de pouvoir regarder le r�sultat du traitement
	cv::waitKey(0);


}