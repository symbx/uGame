//
// Created by symbx on 08.08.17.
//

#include <SFML/Graphics/Sprite.hpp>
#include <utils/Logger.h>
#include <core/Lang.h>
#include "state/StateMenu.h"
#include "core/StateManager.h"
#include "core/Application.h"

namespace uGame {

    StateMenu::StateMenu() {}

    StateMenu::~StateMenu() {
        delete _bgTexture;
        delete _bg;
        delete _menuFrame;
        delete _playBtn;
        delete _profBtn;
        delete _optsBtn;
        delete _exitBtn;
        delete _profile;
    }


    void StateMenu::init(const Application *app) {
        _app = const_cast<Application *>(app);
        _bgTexture = new sf::Texture();
        sf::InputStream *stream = AssetsManager::getStream("/imgs/ui/main_bg.jpg");
        _bgTexture->loadFromStream(*stream);
        _bg = new sf::Sprite();
        _bg->setTexture(*_bgTexture);
        delete stream;
        _menuFrame = new Window(Lang::get(0));
        _playBtn = new Button(Lang::get(1));
        _profBtn = new Button(Lang::get(4));
        _optsBtn = new Button(Lang::get(3));
        _exitBtn = new Button(Lang::get(2));
        _menuFrame->addControl(_playBtn);
        _menuFrame->addControl(_profBtn);
        _menuFrame->addControl(_optsBtn);
        _menuFrame->addControl(_exitBtn);
        _profile = new Label("user@local", static_cast<unsigned int>(_menuFrame->getLocalBounds().width));
        _menuFrame->addControl(_profile);
        centerContent(app->window()->getSize());
    }

    void StateMenu::event(const sf::Event &event) {
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape)
                _app->state()->close();
        }
        if (event.type == sf::Event::Resized) {
            sf::Vector2u wsize(event.size.width, event.size.height);
            centerContent(wsize);
        }
        if(_menuFrame != NULL)
            _menuFrame->event(event);
    }

    void StateMenu::update(const float time) {
        _menuFrame->update(time);
        if(_exitBtn->isPress() || _menuFrame->isClose())
            _app->state()->close();
    }

    void StateMenu::draw(sf::RenderWindow &render, const float time) {
        render.draw(*_bg);
        render.draw(*_menuFrame);
    }

    void StateMenu::centerContent(sf::Vector2u wsize) {
        sf::FloatRect bgsize = _bg->getLocalBounds();
        _bg->setOrigin(bgsize.width/2, bgsize.height/2);
        _bg->setPosition(wsize.x/2, wsize.y/2);
        float ratio = bgsize.width/bgsize.height;
        if((wsize.x/wsize.y) >= ratio) {
            _bg->setScale((bgsize.height/wsize.y)*ratio, bgsize.height/wsize.y);
        }else{
            _bg->setScale(bgsize.width/wsize.x, (bgsize.width/wsize.x)*ratio);
        }

        sf::FloatRect fsize = _menuFrame->getLocalBounds();
        _menuFrame->setPosition((wsize.x-fsize.width)/2, (wsize.y-fsize.height)/2);
        sf::FloatRect btn = _playBtn->getLocalBounds();
        _profile->setPosition(0, 62);
        _playBtn->setPosition((fsize.width - btn.width)/2, 92);
        _profBtn->setPosition((fsize.width - btn.width)/2, 92+btn.height);
        _optsBtn->setPosition((fsize.width - btn.width)/2, 92+btn.height*2);
        _exitBtn->setPosition((fsize.width - btn.width)/2, 92+btn.height*3);
    }
}