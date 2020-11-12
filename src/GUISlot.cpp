#include "GUISlot.hpp"

#include <iostream>
#include <string>
#include <array>
#include <list>
#include <vector>
#include <memory>
#include <algorithm>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "rand.hpp"


bool GUISlot::inited{false};
GLFWwindow* GUISlot::windowPtr{nullptr};

const bool& GUISlot::g_inited() { return GUISlot::inited; }

enum class ActorStat {
	Str,
	Dex,
	Mind,
	Agi,
	Infl,
	End,
	END_OF_LIST

};
enum class ActorBodyPart {
	Head,
	Body,
	Left_Hand,
	Right_Hand,
	Left_Leg,
	Right_Leg,
	END_OF_LIST

};

static const std::array<std::string, static_cast<size_t>(ActorStat::END_OF_LIST)> statsNames{
	"Str",
	"Dex",
	"Mind",
	"Agi",
	"Infl",
	"End"};

const std::string& g_stat_name(const ActorStat& stat_) { return statsNames.at(static_cast<size_t>(stat_));}


enum class ActorAction{
	None,
	Move,
	Move_Contest,
	Attack,
	Shoot,
	Concentration,
	Graple,
	Sweep,
	Dodge,
	Block,
	Shelter,
	Item,
	Item_Contest,
	Remove_Effect,
	Help,
	Special,
	END_OF_LIST
};

ActorAction& operator++(ActorAction &c) {
	if (c == ActorAction::END_OF_LIST) c = static_cast<ActorAction>(0);
	else {
		using IntType = typename std::underlying_type<ActorAction>::type;
		c = static_cast<ActorAction>(static_cast<IntType>(c) + 1);
	}
	return c;
}

ActorAction operator++(ActorAction &c, int) {
	ActorAction result = c;
	++c;
	return result;
}

class ActionsData{
private:
	std::string name{""};
	std::pair<ActorStat, ActorStat> diceStats{ActorStat::END_OF_LIST, ActorStat::END_OF_LIST};
	
	static std::array<ActionsData, static_cast<size_t>(ActorAction::END_OF_LIST)+1> allData;

public:
	ActionsData() = default;
	ActionsData(const std::string& name_, const std::pair<ActorStat, ActorStat>& diceStats_) : name{name_}, diceStats{diceStats_} {} 
	

	const std::string& g_name() const {return this->name; }
	const std::pair<ActorStat, ActorStat>& g_dice_stats() const { return this->diceStats; }

	static const ActionsData& g_data(const ActorAction& action_) { return ActionsData::allData.at(static_cast<size_t>(action_)); }
	static void init(){
		allData.at(static_cast<size_t>(ActorAction::None)) = 			{"---", {ActorStat::END_OF_LIST, ActorStat::END_OF_LIST}};
		allData.at(static_cast<size_t>(ActorAction::Move)) = 			{"Move", {ActorStat::END_OF_LIST, ActorStat::END_OF_LIST}};
		allData.at(static_cast<size_t>(ActorAction::Move_Contest)) = 	{"Move Contest", {ActorStat::Agi, ActorStat::Agi}};
		allData.at(static_cast<size_t>(ActorAction::Attack)) = 			{"Attack", {ActorStat::Str, ActorStat::Dex}};
		allData.at(static_cast<size_t>(ActorAction::Shoot)) = 			{"Shoot", {ActorStat::Dex, ActorStat::Mind}};
		allData.at(static_cast<size_t>(ActorAction::Concentration)) = 	{"Concentration", {ActorStat::Mind, ActorStat::Mind}};
		allData.at(static_cast<size_t>(ActorAction::Graple)) = 			{"Graple", {ActorStat::Str, ActorStat::Dex}};
		allData.at(static_cast<size_t>(ActorAction::Sweep)) = 			{"Sweep", {ActorStat::Str, ActorStat::Dex}};
		allData.at(static_cast<size_t>(ActorAction::Dodge)) = 			{"Dodge", {ActorStat::Dex, ActorStat::Agi}};
		allData.at(static_cast<size_t>(ActorAction::Block)) = 			{"Block", {ActorStat::Str, ActorStat::Dex}};
		allData.at(static_cast<size_t>(ActorAction::Shelter)) = 		{"Shelter", {ActorStat::Agi, ActorStat::Agi}};
		allData.at(static_cast<size_t>(ActorAction::Item)) = 			{"Item", {ActorStat::END_OF_LIST, ActorStat::END_OF_LIST}};
		allData.at(static_cast<size_t>(ActorAction::Item_Contest)) = 	{"Item Contest", {ActorStat::Dex, ActorStat::Agi}};
		allData.at(static_cast<size_t>(ActorAction::Remove_Effect)) = 	{"Remove Effect", {ActorStat::Dex, ActorStat::Dex}};
		allData.at(static_cast<size_t>(ActorAction::Help)) = 			{"Help", {ActorStat::END_OF_LIST, ActorStat::END_OF_LIST}};
		allData.at(static_cast<size_t>(ActorAction::Special)) = 		{"Special", {ActorStat::END_OF_LIST, ActorStat::END_OF_LIST}};
		allData.at(static_cast<size_t>(ActorAction::END_OF_LIST)) = 	{"---", {ActorStat::END_OF_LIST, ActorStat::END_OF_LIST}};
		


	}

};
std::array<ActionsData, static_cast<size_t>(ActorAction::END_OF_LIST)+1> ActionsData::allData;



class ActorSlot : public std::enable_shared_from_this<ActorSlot>{
	friend class std::shared_ptr<ActorSlot>;
private:
	std::string name;
	std::array<int, 6> stats;
	std::list<std::tuple<ActorAction, std::shared_ptr<ActorSlot>, size_t>> actions;
	std::array<std::vector<char>, static_cast<size_t>(ActorBodyPart::END_OF_LIST)> hitPoints;
	std::vector<std::pair<int, int>> rolls; 	// number, amount
	int numberOfDices{0};
	int addInitiative{0};
	int initiative{0};
	bool player{false};
	bool rolled{false};
	bool initChanged{false};
	bool showBody{false};

public:

	ActorSlot(const std::string& name_, const std::array<int, 6>& stats_, const bool& player_, const int& addInitiative_)
		: name{ name_ }, stats{ stats_ }, player{ player_ }{
		for (int i = 0; i < 4; i++) this->g_hit_points(ActorBodyPart::Head).emplace_back(0);
		for (int i = 0; i < 10; i++) this->g_hit_points(ActorBodyPart::Body).emplace_back(0);
		for (int i = 0; i < 5; i++) this->g_hit_points(ActorBodyPart::Head).emplace_back(0);
		for (int i = 0; i < 5; i++) this->g_hit_points(ActorBodyPart::Right_Hand).emplace_back(0);
		for (int i = 0; i < 5; i++) this->g_hit_points(ActorBodyPart::Right_Leg).emplace_back(0);
		for (int i = 0; i < 5; i++) this->g_hit_points(ActorBodyPart::Left_Hand).emplace_back(0);
		for (int i = 0; i < 5; i++) this->g_hit_points(ActorBodyPart::Left_Leg).emplace_back(0);
		this->set_number_of_actions();
		this->calc_initiative();
	}
	const int& g_initiative() const { return this->initiative; } 
	const std::string& g_name() const { return this->name; }
	const std::array<int, 6>& g_stats() const { return this->stats; }
	const int& g_stats(const ActorStat& stat_) const { return this->stats.at(static_cast<size_t>(stat_)); }
	const bool& is_player() const { return this->player; }
	const bool& g_rolled() const { return this->rolled; }
	const bool& g_show_body() const { return this->showBody; }
	std::list<std::tuple<ActorAction, std::shared_ptr<ActorSlot>, size_t>>& g_actions() { return this->actions; }
	std::vector<std::pair<int, int>>& g_rolls() { return this->rolls; }
	std::array<std::vector<char>, static_cast<size_t>(ActorBodyPart::END_OF_LIST)>& g_hit_points() { return this->hitPoints; }
	std::vector<char>& g_hit_points(const ActorBodyPart& part_) { return this->hitPoints.at(static_cast<size_t>(part_)); }
	
	void set_show_body(const bool& var_) { this->showBody = var_; }

	void change_additional_initiative(const int& value_) { this->addInitiative = value_; this->calc_initiative();}

	void calc_initiative() { 
		int tempInit = this->initiative;
		this->initiative = (this->stats.at(static_cast<size_t>(ActorStat::Mind))*2) + this->addInitiative; 
		if (tempInit != 0 && tempInit != this->initiative) this->initChanged = true;
	}
	
	void set_number_of_actions(){
		const int requestedActions = std::min(this->g_stats(ActorStat::Dex), 8);
		if (requestedActions < 0) return;
		while (this->actions.size() != requestedActions){
			if (this->actions.size() < requestedActions) this->actions.push_back(std::make_tuple(ActorAction::END_OF_LIST, nullptr, 100));
			else this->actions.pop_back();
		}
	}
	void calculate_number_of_dices(){
		this->numberOfDices = 100;
		for (const auto& Fi : this->actions){
			if (std::get<0>(Fi) >= ActorAction::END_OF_LIST) continue;
			if (ActionsData::g_data(std::get<0>(Fi)).g_dice_stats().first >= ActorStat::END_OF_LIST || ActionsData::g_data(std::get<0>(Fi)).g_dice_stats().second >= ActorStat::END_OF_LIST) continue;
			int tempDices = this->g_stats(ActionsData::g_data(std::get<0>(Fi)).g_dice_stats().first) + this->g_stats(ActionsData::g_data(std::get<0>(Fi)).g_dice_stats().second);
			if (tempDices < this->numberOfDices) this->numberOfDices = tempDices;
		}
		if (this->numberOfDices == 100) this->numberOfDices = 0;
	}
	void roll(){
		this->rolls.clear();
		for (int i = 0; i < this->numberOfDices; i++){
			int tempRand = rand_int(1,10);
			bool tempFind = false;
			for (auto& Fi : this->rolls) { if (tempRand == Fi.first) { Fi.second++; tempFind = true; break; }}
			if (!tempFind) this->rolls.emplace_back(std::make_pair(tempRand, 1));
		}
		this->rolled = true;
		for (auto& Fi : this->actions) { std::get<2>(Fi) = 100; }
	}
	void new_turn(){
		this->rolled=false;
		this->actions.clear();
		this->rolls.clear();
		this->set_number_of_actions();
	}
	void add_hp(const int& direction_, const int& amount_, const bool& heal_){
		int tempDir{-1};
		switch(direction_){
			case 1:	break;
			case 2:	break;
			case 3:	break;
			case 4:	break;
			case 5:	break;
			case 6:	break;
			case 7:	break;
			case 8:	break;
			case 9:	break;
			case 10:break;
			default:
				break;

		}
		if (tempDir < 0 || tempDir > 5) return;
		for (auto& Fi : this->hitPoints.at(tempDir)) {
			

		}
	}





};

static std::list<std::shared_ptr<ActorSlot>> allCreatures;



void GUISlot::init(GLFWwindow* window_){
	if(!GUISlot::inited){
		if (window_ == nullptr) return;
		// Initialize OpenGL loader
	#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
		bool err = gl3wInit() != 0;
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
		bool err = glewInit() != GLEW_OK;
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
		bool err = gladLoadGL() == 0;
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
		bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
		bool err = false;
		glbinding::Binding::initialize();
	#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
		bool err = false;
		glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
	#else
		bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
	#endif
		if (err)
		{
			std::cout << "IMGUI: Failed to initialize OpenGL loader!\n";
			return;
		}


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window_, true);
	ImGui_ImplOpenGL3_Init("#version 130");


		ActionsData::init();
		GUISlot::windowPtr = window_;
		GUISlot::inited = true;    

	}

	
}

void GUISlot::destroy(){
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

}


void print_creature(const std::shared_ptr<ActorSlot>& crea_, const bool& oneLine_){
	ImGui::TextWrapped("Name:%s", crea_->g_name().c_str());
	if (oneLine_) ImGui::SameLine();
	ImGui::TextWrapped("Init:%i", crea_->g_initiative());

	for(int i = 0; i < crea_->g_stats().size(); i++){
		if(oneLine_ || i%2 == 1) ImGui::SameLine();
		ImGui::TextWrapped("%s:%i", statsNames.at(i).c_str(), crea_->g_stats().at(i));
	}
	
}

void print_hp(const std::shared_ptr<ActorSlot>& crea_, const ImVec2& offset_ = {0.f,0.f}){
	try {
		for (int i = 0; i < 4; i++) {
			ImGui::PushID(i + static_cast<int>(ActorBodyPart::Head)*100);
			ImGui::SetCursorPos(ImVec2{45.f + (15.f*(i%2)), 10.f + 15.f*(i/2)}+offset_);
			switch (crea_->g_hit_points(ActorBodyPart::Head).at(i)){
			case 0:	if (ImGui::SmallButton(" ")) crea_->g_hit_points(ActorBodyPart::Head).at(i) = 1; break;
			case 1:	if (ImGui::SmallButton("/")) crea_->g_hit_points(ActorBodyPart::Head).at(i) = 2; break;
			default: if (ImGui::SmallButton("X")) crea_->g_hit_points(ActorBodyPart::Head).at(i) = 0; break;
			}
			ImGui::PopID();
		}
		for (int i = 0; i < 10; i++) {
			ImGui::PushID(i + static_cast<int>(ActorBodyPart::Body) * 100);
			ImGui::SetCursorPos(ImVec2{30.f + (15.f*(i%4)), 40.f + 15.f*(i/4)}+offset_);
			switch (crea_->g_hit_points(ActorBodyPart::Body).at(i)) {
			case 0:	if (ImGui::SmallButton(" ")) crea_->g_hit_points(ActorBodyPart::Body).at(i) = 1; break;
			case 1:	if (ImGui::SmallButton("/")) crea_->g_hit_points(ActorBodyPart::Body).at(i) = 2; break;
			default: if (ImGui::SmallButton("X")) crea_->g_hit_points(ActorBodyPart::Body).at(i) = 0; break;
			}
			ImGui::PopID();
		}
		for (int i = 0; i < 5; i++) {
			ImGui::PushID(i + static_cast<int>(ActorBodyPart::Right_Hand) * 100);
			ImGui::SetCursorPos(ImVec2{10.f , 40.f + 15.f*i}+offset_);
			switch (crea_->g_hit_points(ActorBodyPart::Right_Hand).at(i)) {
			case 0:	if (ImGui::SmallButton(" ")) crea_->g_hit_points(ActorBodyPart::Right_Hand).at(i) = 1; break;
			case 1:	if (ImGui::SmallButton("/")) crea_->g_hit_points(ActorBodyPart::Right_Hand).at(i) = 2; break;
			default: if (ImGui::SmallButton("X")) crea_->g_hit_points(ActorBodyPart::Right_Hand).at(i) = 0; break;
			}
			ImGui::PopID();
		}
		for (int i = 0; i < 5; i++) {
			ImGui::PushID(i + static_cast<int>(ActorBodyPart::Right_Leg) * 100);
			ImGui::SetCursorPos(ImVec2{95.f, 40.f + 15.f*i}+offset_);
			switch (crea_->g_hit_points(ActorBodyPart::Right_Leg).at(i)) {
			case 0:	if (ImGui::SmallButton(" ")) crea_->g_hit_points(ActorBodyPart::Right_Leg).at(i) = 1; break;
			case 1:	if (ImGui::SmallButton("/")) crea_->g_hit_points(ActorBodyPart::Right_Leg).at(i) = 2; break;
			default: if (ImGui::SmallButton("X")) crea_->g_hit_points(ActorBodyPart::Right_Leg).at(i) = 0; break;
			}
			ImGui::PopID();
		}
		for (int i = 0; i < 5; i++) {
			ImGui::PushID(i + static_cast<int>(ActorBodyPart::Left_Hand) * 100);
			ImGui::SetCursorPos(ImVec2{35.f , 85.f + 15.f*i}+offset_);
			switch (crea_->g_hit_points(ActorBodyPart::Left_Hand).at(i)) {
			case 0:	if (ImGui::SmallButton(" ")) crea_->g_hit_points(ActorBodyPart::Left_Hand).at(i) = 1; break;
			case 1:	if (ImGui::SmallButton("/")) crea_->g_hit_points(ActorBodyPart::Left_Hand).at(i) = 2; break;
			default: if (ImGui::SmallButton("X")) crea_->g_hit_points(ActorBodyPart::Left_Hand).at(i) = 0; break;
			}
			ImGui::PopID();
		}
		for (int i = 0; i < 5; i++) {
			ImGui::PushID(i + static_cast<int>(ActorBodyPart::Left_Leg) * 100);
			ImGui::SetCursorPos(ImVec2{70.f, 85.f + 15.f*i}+offset_);
			switch (crea_->g_hit_points(ActorBodyPart::Left_Leg).at(i)) {
			case 0:	if (ImGui::SmallButton(" ")) crea_->g_hit_points(ActorBodyPart::Left_Leg).at(i) = 1; break;
			case 1:	if (ImGui::SmallButton("/")) crea_->g_hit_points(ActorBodyPart::Left_Leg).at(i) = 2; break;
			default: if (ImGui::SmallButton("X")) crea_->g_hit_points(ActorBodyPart::Left_Leg).at(i) = 0; break;
			}
			ImGui::PopID();
		}
	}
	catch(std::exception e_){
		

	}

}

void print_tooltip(const std::shared_ptr<ActorSlot>& crea_){
	ImGui::BeginTooltip();
	print_hp(crea_);
	//TODO
	ImGui::EndTooltip();

}

void manage_creatures(const bool& players_){

	static bool cleanAfterPush{true};
	static char tempName[40];
	static int tempInitiative{0};
	static std::array<int, static_cast<size_t>(ActorStat::END_OF_LIST)> tempStats{0,0,0,0,0,0};
	ImGuiInputTextFlags inputFlags = 0;
	inputFlags |= ImGuiInputTextFlags_CharsDecimal;
	inputFlags |= ImGuiInputTextFlags_CharsNoBlank;

    ImGui::InputText("Name", tempName, IM_ARRAYSIZE(tempName));
	ImGui::SameLine();
	ImGui::Checkbox("Clean after push", &cleanAfterPush);

	ImGui::InputInt("Additional Initiative", &tempInitiative);

	ImGui::Separator();
	for (int i = 0; i < 6; i++) ImGui::InputInt(statsNames.at(i).c_str(), &tempStats[i]);
	
	ImGui::Separator();
	if (ImGui::Button("Randomize Stats")){
		for(int i = 0; i < 6; i++){
			int tempRandStat = rand_int(1,10);
			switch (tempRandStat){
			case 1: 
			case 2: tempStats[i] = 2; break;
			case 3:
			case 4: 
			case 5: tempStats[i] = 3; break;
			case 6: 
			case 7: 
			case 8: tempStats[i] = 4; break;
			case 9: 
			case 10:tempStats[i] = 5; break;
			default:break;}
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Push Actor")) {
		if (tempName[0] != '\0'){
			bool tempPushed{false};
			auto tempActor = std::make_shared<ActorSlot>(tempName, tempStats, players_, tempInitiative);
			for (auto Fi = allCreatures.begin(); Fi != allCreatures.end(); ++Fi) {
				if(tempActor->g_initiative() < (*Fi)->g_initiative() || (tempActor->g_initiative() <= (*Fi)->g_initiative() && !(*Fi)->is_player())){
					allCreatures.emplace(Fi, tempActor);
					tempPushed = true;
					break;
				}
			}
			if(!tempPushed) allCreatures.emplace_back(tempActor);
			if(cleanAfterPush){
				memset(tempName, 0, IM_ARRAYSIZE(tempName));
				tempInitiative = 0;
				tempStats = {0,0,0,0,0,0};
			}
		}
	}

	if (ImGui::BeginChild("List", ImVec2(0.f, 0.f), true, 0)){
		int id = 0;
		for (auto Fi = allCreatures.begin(); Fi != allCreatures.end();) {
			if (players_ == (*Fi)->is_player()) {
				ImGui::PushID(id);
				if (ImGui::Button("-")) {
					Fi = allCreatures.erase(Fi);
					ImGui::PopID();
					id++;
					continue;
				}
				ImGui::SameLine();
				ImGui::BeginGroup();
				print_creature(*Fi, true);
				ImGui::EndGroup();
				if(ImGui::IsItemHovered()){
					print_tooltip(*Fi);
				}
				ImGui::PopID();
				id++;
			}
			++Fi;
		}
		ImGui::EndChild();
	}
}


void game_menu(){
	ImGui::SetNextWindowContentSize({allCreatures.size() * 260.f, 700.f});
	if(ImGui::BeginChild("AllActors", ImVec2(0.f, ImGui::GetWindowSize().y/1.2f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysHorizontalScrollbar)){
		ImGui::Columns(allCreatures.size() > 0 ? allCreatures.size() : 1);
		for (auto& Fi : allCreatures){ 
			ImGui::PushID(Fi.get());
			ImGui::BeginGroup();
			print_creature(Fi, false);
			int notFirst2{0};
			ImGui::Text("---------------------------");
			ImGui::TextWrapped("Actions:");
			for (auto& Ai : Fi->g_actions()) {
				ImGui::PushID(notFirst2);
				if (notFirst2%2 == 1) ImGui::SameLine();
				else ImGui::Text("---------------------------");
				ImGui::BeginGroup();

				ImGui::SetNextItemWidth(100.f);
				if (ImGui::BeginCombo("##Actions", ActionsData::g_data(std::get<0>(Ai)).g_name().c_str(), ImGuiComboFlags_NoArrowButton)) {
            		for (ActorAction i = static_cast<ActorAction>(0); i < ActorAction::END_OF_LIST; i++) {
               			const bool is_selected = (std::get<0>(Ai) == i);
                		if (ImGui::Selectable(ActionsData::g_data(i).g_name().c_str(), is_selected)) {
							std::get<0>(Ai) = i;
							std::get<1>(Ai) = nullptr;
							std::get<2>(Ai) = 100;
							Fi->calculate_number_of_dices();
						} 
                		if (is_selected) ImGui::SetItemDefaultFocus();
            		}
            		ImGui::EndCombo();
        		}

				ImGui::SetNextItemWidth(100.f);
				if (ImGui::BeginCombo("##Targets", (std::get<1>(Ai) ? std::get<1>(Ai)->g_name().c_str() : "---"), ImGuiComboFlags_NoArrowButton)) {
            		if (std::get<0>(Ai) > ActorAction::None && std::get<0>(Ai) < ActorAction::END_OF_LIST) for (auto& i : allCreatures) {
						if(!i) continue;
               			const bool is_selected = (std::get<1>(Ai) == i);
                		if (ImGui::Selectable(i->g_name().c_str(), is_selected)) {
							std::get<1>(Ai) = i;
							
						} 
                		if (is_selected) ImGui::SetItemDefaultFocus();
            		}
            		ImGui::EndCombo();
        		}

				ImGui::SetNextItemWidth(100.f);
				std::string tempLableForDice = (std::get<2>(Ai) < Fi->g_rolls().size() ? (std::to_string(Fi->g_rolls().at(std::get<2>(Ai)).first) + ": " + std::to_string(Fi->g_rolls().at(std::get<2>(Ai)).second)) : "---");
				if (ImGui::BeginCombo("##Dice", tempLableForDice.c_str(), ImGuiComboFlags_NoArrowButton)) {
					bool is_selected = false;
					if (std::get<0>(Ai) > ActorAction::None && std::get<0>(Ai) < ActorAction::END_OF_LIST) for (size_t i = 0; i < Fi->g_rolls().size(); i++) {
						if (Fi->g_rolls().at(i).second < 2) continue; 
						if(std::any_of(Fi->g_actions().begin(), Fi->g_actions().end(), [&](const auto& tuple_){ return ((std::get<2>(tuple_) == i) && (std::get<2>(tuple_) <= 10)); })) continue;
						is_selected = (std::get<2>(Ai) == i);
						std::string tempPairDiceString = (std::to_string(Fi->g_rolls().at(i).first) + ": " + std::to_string(Fi->g_rolls().at(i).second));
                		if (ImGui::Selectable(tempPairDiceString.c_str(), is_selected)) {
							std::get<2>(Ai) = i;
						} 
                		if (is_selected) ImGui::SetItemDefaultFocus();
					}
					is_selected = (std::get<2>(Ai) > 10);
					if (ImGui::Selectable("---", is_selected)){
						std::get<2>(Ai) = 100;
					}
					if (is_selected) ImGui::SetItemDefaultFocus();
            		ImGui::EndCombo();
        		}
				ImGui::EndGroup();
				notFirst2++;
				ImGui::PopID();
			}
			{
				int tempI = 1;
				ImGui::Text("---------------------------");
				ImGui::Text("Rolls:");
				ImGui::BeginGroup();
				for (const auto& Ri : Fi->g_rolls()){
					if (tempI%3 != 0) ImGui::SameLine();
					ImGui::TextWrapped(" %i:%i,", Ri.first, Ri.second);
					tempI++;
				}
				ImGui::EndGroup();
			}
			ImGui::Text("---------------------------");
			if (ImGui::Button("Clear")) Fi->new_turn();
			ImGui::SameLine();
			if (ImGui::Button("Roll")) Fi->roll();
			ImGui::SameLine();
			if (ImGui::Button("Show Body")) Fi->set_show_body(!Fi->g_show_body());

			if (Fi->g_show_body()) {
				print_hp(Fi, ImGui::GetCursorPos());
			}

			
			ImGui::EndGroup();
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);
		ImGui::EndChild();
	}

	if(ImGui::Button("Roll All")){
		for(auto& Fi : allCreatures){ if(!Fi->g_rolled()) Fi->roll(); }
	}
	ImGui::SameLine();

	if(ImGui::Button("Roll Enemies")){
		for(auto& Fi : allCreatures){ if (!Fi->is_player()) if (!Fi->g_rolled()) Fi->roll(); }
	}
	ImGui::SameLine();

	if(ImGui::Button("Roll Players")){
		for(auto& Fi : allCreatures){ if (Fi->is_player()) if (!Fi->g_rolled()) Fi->roll(); }
	}
	ImGui::SameLine();

	if(ImGui::Button("Next Turn")){
		for(auto& Fi : allCreatures) Fi->new_turn();
	}
	

}

void GUISlot::draw(){
	

	ImGuiWindowFlags window_flags = 0;

	if (!GUISlot::windowPtr) return;
	if (!GUISlot::inited) return;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	int display_w, display_h;
	glfwGetFramebufferSize(GUISlot::windowPtr, &display_w, &display_h);

	
	{
		
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		window_flags |= ImGuiWindowFlags_NoScrollbar;
    	window_flags |= ImGuiWindowFlags_NoMove;
   	 	window_flags |= ImGuiWindowFlags_NoResize;
    	window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoResize;
		ImGui::Begin("Main", nullptr, window_flags);
		ImGui::SetWindowSize({static_cast<float>(display_w), static_cast<float>(display_h)});
		ImGui::SetWindowPos({0.f, 0.f});

		if (ImGui::BeginTabBar("All", 0)) {
            if (ImGui::BeginTabItem("Manage Enemies")) {
					manage_creatures(false);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Manage Players")) {
					manage_creatures(true);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Game")){
                    game_menu();
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

		
		
		ImGui::End();
	}
	

	
	// Rendering
	ImGui::Render();
   
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
}
