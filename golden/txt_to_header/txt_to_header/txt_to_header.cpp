#include <bits/stdc++.h>
using namespace std;

class DNN_layer_sizes {
private:
    // Quantization scaling factor
    const int input_scale = 127;
    const int conv1_output_scale = 2221;
    const int conv2_output_scale = 533;
    const int fc1_output_scale = 297;
    const int fc2_output_scale = 388;
    const int fc3_output_scale = 366;


    // weights size
    const vector<int> conv1_weight_size = { 6, 3, 5, 5 };
    const vector<int> conv2_weight_size = { 16, 6, 5, 5 };
    const vector<int> fc1_weight_size = { 120, 400 };
    const vector<int> fc2_weight_size = { 84, 120 };
    const vector<int> fc3_weight_size = { 10, 84 };
    const vector<int> fc3_bias_size = { 10};

    // input
    const vector<int> input_quantized_size = { 3, 32, 32 };

    // Conv size
    const vector<int> conv1_activation_size = { 6,28,28 };
    const vector<int> conv1_max_pool_size = { 6,14,14 };
    const vector<int> conv1_quantized_result_size = { 6,14,14 }; //quantized, also be the input of next layer

    const vector<int> conv2_activation_size = { 16,10,10 };
    const vector<int> conv2_max_pool_size = { 16,5,5 };
    const vector<int> conv2_quantized_result_size = { 16,5,5 };//quantized, also be the input of next layer

    // flatten activation, 16*5*5 -> 400
    const vector<int> fc1_input_size = { 400 };

    // FC size
    const vector<int> fc1_activation_size = { 120 };
    const vector<int> fc1_quantized_result_size = { 120 };
    const vector<int> fc2_activation_size = { 84 };
    const vector<int> fc2_quantized_result_size = { 84 };
    const vector<int> fc3_activation_size = { 10 };
    const vector<int> fc3_bias_addition_size = { 10 };
    const vector<int> fc3_quantized_result_size = { 10 };

    int file_num;

public:
    DNN_layer_sizes(int num) {
        file_num = num;
    }
    void write_include_header() {
        fstream file;

        file.open("generated_header/DNN_parameters.h", ios::out);
        file << "#ifndef _DNN_PARAMETERS_DEF_H_" << endl;
        file << "#define _DNN_PARAMETERS_DEF_H_" << endl << endl;

        file << "#include \"scale.h\"" << endl;
        file << "#include \"conv1_weights.h\"" << endl;
        file << "#include \"conv2_weights.h\"" << endl;
        file << "#include \"fc1_weights.h\"" << endl;
        file << "#include \"fc2_weights.h\"" << endl;
        file << "#include \"fc3_weights.h\"" << endl;
        file << "#include \"fc3_bias.h\"" << endl;
        file << "#include \"input_quantized_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"conv1_activation_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"conv1_max_pool_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"conv1_quantized_result_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"conv2_activation_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"conv2_max_pool_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"conv2_quantized_result_" + to_string(file_num) + ".h\"" << endl;

        file << "#include \"fc1_input_" + to_string(file_num) + ".h\"" << endl;

        file << "#include \"fc1_activation_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"fc1_quantized_result_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"fc2_activation_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"fc2_quantized_result_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"fc3_activation_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"fc3_bias_addition_" + to_string(file_num) + ".h\"" << endl;
        file << "#include \"fc3_quantized_result_" + to_string(file_num) + ".h\"" << endl;


        file << endl << "#endif" << endl;

    }

    void write_scale() {
        fstream file;
        file.open("generated_header/scale.h", ios::out);
        file << "#ifndef _SCALE_DEF_H_" << endl;
        file << "#define _SCALE_DEF_H_" << endl << endl;

        file << "const int input_scale = " << input_scale << ";" << endl;
        file << "const int conv1_output_scale = " << conv1_output_scale << ";" << endl;
        file << "const int conv2_output_scale = " << conv2_output_scale << ";" << endl;
        file << "const int fc1_output_scale = " << fc1_output_scale << ";" << endl;
        file << "const int fc2_output_scale = " << fc2_output_scale << ";" << endl;
        file << "const int fc3_output_scale = " << fc3_output_scale << ";" << endl;

        file << endl << "#endif" << endl;
        file.close();
    }

    void write_weights(){
        write_4D_data("conv1_weights", conv1_weight_size);
        write_4D_data("conv2_weights", conv2_weight_size);
        write_2D_data("fc1_weights", fc1_weight_size);
        write_2D_data("fc2_weights", fc2_weight_size);
        write_2D_data("fc3_weights", fc3_weight_size);
        write_1D_data("fc3_bias", fc3_bias_size);
    }

    void write_data() {


        write_3D_data("input_quantized_" + to_string(file_num), input_quantized_size);

        write_3D_data("conv1_activation_" + to_string(file_num), conv1_activation_size);
        write_3D_data("conv1_max_pool_" + to_string(file_num), conv1_max_pool_size);
        write_3D_data("conv1_quantized_result_" + to_string(file_num), conv1_quantized_result_size);

        write_3D_data("conv2_activation_" + to_string(file_num), conv2_activation_size);
        write_3D_data("conv2_max_pool_" + to_string(file_num), conv2_max_pool_size);
        write_3D_data("conv2_quantized_result_" + to_string(file_num), conv2_quantized_result_size);

        write_1D_data("fc1_input_" + to_string(file_num), fc1_input_size);

        write_1D_data("fc1_activation_" + to_string(file_num), fc1_activation_size);
        write_1D_data("fc1_quantized_result_" + to_string(file_num), fc1_quantized_result_size);

        write_1D_data("fc2_activation_" + to_string(file_num), fc2_activation_size);
        write_1D_data("fc2_quantized_result_" + to_string(file_num), fc2_quantized_result_size);

        write_1D_data("fc3_activation_" + to_string(file_num), fc3_activation_size);
        write_1D_data("fc3_bias_addition_" + to_string(file_num), fc3_bias_addition_size);
        write_1D_data("fc3_quantized_result_" + to_string(file_num), fc3_quantized_result_size);
    }
    void write_4D_data(string filename, vector<int> file_size) {
        fstream file;
        vector<int> data;
        int temp;
        file.open("weights/" + filename + ".txt", ios::in);
        while (!file.eof()) {
            file >> temp;
            data.emplace_back(temp);

        }

        file.close();

        temp = 0;
        file.open("generated_header/" + filename + ".h", ios::out);
        string DEFINE = filename;
        for (int i = 0; i < DEFINE.size(); i++)
            if (97 <= DEFINE[i] && DEFINE[i] <= 122)
                DEFINE[i] -= 32;

        file << "#ifndef _" + DEFINE + "_DEF_H_" << endl;
        file << "#define _" + DEFINE + "_DEF_H_" << endl << endl;

        file << "const int " + filename;
        for (int index : file_size)
            file << "[" << index << "]";
        file << " = {" << endl;
        
        for (int i = 0; i < file_size[0]; i++) {
            file << "   {" << endl;
            for (int j = 0; j < file_size[1]; j++) {
                file << "       {" << endl;
                for (int k = 0; k < file_size[2]; k++) {
                    file << "           {";
                    for (int l = 0; l < file_size[3]; l++) {
                        file << data[temp++] << (l == file_size[3] - 1 ? "" : ",");
                    }
                    file << "\t}" << (k == file_size[2] - 1 ? "" : ",") << endl;
                }
                file << "       }" << (j == file_size[1] - 1 ? "" : ",") << endl;
            }
            file << "   }" << (i == file_size[0] - 1 ? "" : ",") << endl;
        }
        file << "};" << endl;

        file << endl << "#endif" << endl;
        cout << "file: " << filename << " write " << temp << " numbers" << endl;
        file.close();
    }

    void write_3D_data(string filename, vector<int> file_size) {
        fstream file;
        vector<int> data;
        int temp;
        file.open("weights/" + filename + ".txt", ios::in);
        while (!file.eof()) {
            file >> temp;
            data.emplace_back(temp);

        }

        file.close();

        temp = 0;
        file.open("generated_header/" + filename + ".h", ios::out);
        string DEFINE = filename;
        for (int i = 0; i < DEFINE.size(); i++)
            if (97 <= DEFINE[i] && DEFINE[i] <= 122)
                DEFINE[i] -= 32;

        file << "#ifndef _" + DEFINE + "_DEF_H_" << endl;
        file << "#define _" + DEFINE + "_DEF_H_" << endl << endl;

        file << "const int " + filename;
        for (int index : file_size)
            file << "[" << index << "]";
        file << " = {" << endl;

            for (int j = 0; j < file_size[0]; j++) {
                file << "       {" << endl;
                for (int k = 0; k < file_size[1]; k++) {
                    file << "           {";
                    for (int l = 0; l < file_size[2]; l++) {
                        file << data[temp++] << (l == file_size[2] - 1 ? "" : ",");
                    }
                    file << "\t}" << (k == file_size[1] - 1 ? "" : ",") << endl;
                }
                file << "       }" << (j == file_size[0] - 1 ? "" : ",") << endl;
            }
        file << "};" << endl;

        file << endl << "#endif" << endl;
        cout << "file: " << filename << " write " << temp << " numbers" << endl;
        file.close();
    }
    void write_2D_data(string filename, vector<int> file_size) {
        fstream file;
        vector<int> data;
        int temp;
        file.open("weights/" + filename + ".txt", ios::in);
        while (!file.eof()) {
            file >> temp;
            data.emplace_back(temp);
        }

        file.close();

        temp = 0;
        file.open("generated_header/" + filename + ".h", ios::out);
        string DEFINE = filename;
        for (int i = 0; i < DEFINE.size(); i++)
            if (97 <= DEFINE[i] && DEFINE[i] <= 122)
                DEFINE[i] -= 32;

        file << "#ifndef _" + DEFINE + "_DEF_H_" << endl;
        file << "#define _" + DEFINE + "_DEF_H_" << endl << endl;

        file << "const int " + filename;
        for (int index : file_size)
            file << "[" << index << "]";
        file << " = {" << endl;

        for (int i = 0; i < file_size[0]; i++) {
            file << "   {" << endl << "     ";
            for (int j = 0; j < file_size[1]; j++) {
                
                file  << data[temp++];

                if (j == file_size[1] - 1)
                    file << "\n";
                else if (j != 0 && j % 10 == 0)
                    file << ",\n        ";
                else
                    file << ", ";
            }
            file << "   }" << (i == file_size[0] - 1 ? "" : ",") << endl;
        }
        file << "};" << endl;

        file << endl << "#endif" << endl;
        cout << "file: " << filename << " write " << temp << " numbers" << endl;
        file.close();
    }

    void write_1D_data(string filename, vector<int> file_size) {
        fstream file;
        vector<int> data;
        int temp;
        file.open("weights/" + filename + ".txt", ios::in);
        while (!file.eof()) {
            file >> temp;
            data.emplace_back(temp);

        }

        file.close();

        temp = 0;
        file.open("generated_header/" + filename + ".h", ios::out);
        string DEFINE = filename;
        for (int i = 0; i < DEFINE.size(); i++)
            if (97 <= DEFINE[i] && DEFINE[i] <= 122)
                DEFINE[i] -= 32;

        file << "#ifndef _" + DEFINE + "_DEF_H_" << endl;
        file << "#define _" + DEFINE + "_DEF_H_" << endl << endl;

        file << "const int " + filename;
        for (int index : file_size)
            file << "[" << index << "]";
        file << " = {" << endl << "     ";


        for (int l = 0; l < file_size[0]; l++) {
            file << data[temp++] << (l == file_size[0] - 1 ? "\n" : ",");
        }

        file << "};" << endl;
        file << endl << "#endif" << endl;
        cout << "file: " << filename << " write " << temp << " numbers" << endl;
        file.close();
    }
};


string dec_to_hex(int num) {
    string ans = "0x";
    string hex[16] = { "0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f" };
    ans += hex[num / 16];
    ans += hex[num % 16];

    return ans;
}
int main() {
   
    DNN_layer_sizes para(1);
    para.write_scale();
    para.write_weights();
    para.write_data();
    para.write_include_header();

    
    
    return 0;
}



